#!/usr/bin/env python3
"""live-preview.py — precision UI verification loop via slint-lsp.

Boots slint-lsp over stdio, asks it to live-preview any component from any
.slint file, sizes the preview window to an exact client rect (DPI-corrected),
and captures the window to PNG on demand. Re-capturing after an edit requires
NO rebuild — the LSP watches the file and re-renders instantly.

Usage:
  python scripts/live-preview.py preview FILE.slint COMPONENT [--size WxH]
  python scripts/live-preview.py capture OUT.png
  python scripts/live-preview.py resize WxH
  python scripts/live-preview.py size
  python scripts/live-preview.py stop

The LSP process stays alive between invocations; state (PID, ports) is kept
in build/.live-preview.json. Send a JSON-RPC request via a small control
server on localhost so each CLI invocation talks to the same driver.

Simplified design: run this script with no arguments to start an interactive
server on localhost:42123 that accepts JSON commands:
  {"cmd": "preview", "file": "...", "component": "...", "width": 1280, "height": 800}
  {"cmd": "capture", "out": "shot.png"}
  {"cmd": "stop"}
"""

import ctypes
import ctypes.wintypes as wt
import json
import os
import socket
import subprocess
import sys
import threading
import time
import base64
import struct
import zlib

HOST = "127.0.0.1"
PORT = 42123
MCP_PORT = 42124  # Slint runtime's own MCP server (SLINT_MCP_PORT)
STATE_FILE = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "build", ".live-preview.json")
LSP_EXE = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    ".deps-cache", "AMD64", "slint-src", "target", "release", "slint-lsp.exe",
)

user32 = ctypes.windll.user32
kernel32 = ctypes.windll.kernel32
gdi32 = ctypes.windll.gdi32

# DPI awareness so SetWindowPos math is in real pixels
try:
    user32.SetProcessDpiAwarenessContext(ctypes.c_void_p(-4))  # PER_MONITOR_AWARE_V2
except Exception:
    try:
        ctypes.windll.shcore.SetProcessDpiAwareness(2)
    except Exception:
        user32.SetProcessDPIAware()


def find_window_by_pid(pid, timeout=15.0):
    """Return HWND of the preview window owned by pid (visible top-level)."""
    deadline = time.time() + timeout
    result = []

    @ctypes.WINFUNCTYPE(ctypes.c_bool, wt.HWND, wt.LPARAM)
    def enum_proc(hwnd, lparam):
        win_pid = wt.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(win_pid))
        if win_pid.value == pid and user32.IsWindowVisible(hwnd):
            cls = ctypes.create_unicode_buffer(256)
            user32.GetClassNameW(hwnd, cls, 256)
            # skip tool windows / menus
            if cls.value not in ("tooltips_class32", "MSICTLMsgWnd"):
                result.append(hwnd)
        return True

    while time.time() < deadline:
        result.clear()
        user32.EnumWindows(enum_proc, 0)
        if result:
            return result[0]
        time.sleep(0.25)
    return None


def get_client_rect(hwnd):
    rect = wt.RECT()
    user32.GetClientRect(hwnd, ctypes.byref(rect))
    return rect.right - rect.left, rect.bottom - rect.top


def resize_client(hwnd, w, h):
    """Resize so the CLIENT area is exactly w x h (2-pass to absorb menu/DPI)."""
    for _ in range(2):
        cw, ch = get_client_rect(hwnd)
        if cw == w and ch == h:
            return True
        # compute deltas and grow the outer rect
        outer = wt.RECT()
        user32.GetWindowRect(hwnd, ctypes.byref(outer))
        dw = (outer.right - outer.left) - cw
        dh = (outer.bottom - outer.top) - ch
        user32.SetWindowPos(hwnd, 0, outer.left, outer.top, w + dw, h + dh, 0x0004)  # SWP_NOZORDER
        time.sleep(0.05)
    cw, ch = get_client_rect(hwnd)
    return cw == w and ch == h


def capture_window_png(hwnd, out_path):
    """BitBlt the window's screen pixels into a PNG (no external deps)."""
    cw, ch = get_client_rect(hwnd)
    if cw <= 0 or ch <= 0:
        raise RuntimeError("window has empty client rect")

    pt = wt.POINT(0, 0)
    user32.ClientToScreen(hwnd, ctypes.byref(pt))

    hdc_screen = user32.GetDC(0)  # screen DC: source coords are screen coords
    hdc_window = user32.GetDC(hwnd)
    hdc_mem = gdi32.CreateCompatibleDC(hdc_window)
    hbmp = gdi32.CreateCompatibleBitmap(hdc_window, cw, ch)
    old = gdi32.SelectObject(hdc_mem, hbmp)
    gdi32.BitBlt(hdc_mem, 0, 0, cw, ch, hdc_screen, pt.x, pt.y, 0x00CC0020)  # SRCCOPY
    gdi32.SelectObject(hdc_mem, old)

    # --- read pixels into a BGRA buffer ---
    class BITMAPINFOHEADER(ctypes.Structure):
        _fields_ = [
            ("biSize", wt.DWORD), ("biWidth", wt.LONG), ("biHeight", wt.LONG),
            ("biPlanes", wt.WORD), ("biBitCount", wt.WORD), ("biCompression", wt.DWORD),
            ("biSizeImage", wt.DWORD), ("biXPelsPerMeter", wt.LONG), ("biYPelsPerMeter", wt.LONG),
            ("biClrUsed", wt.DWORD), ("biClrImportant", wt.DWORD),
        ]

    bmi = BITMAPINFOHEADER()
    bmi.biSize = ctypes.sizeof(BITMAPINFOHEADER)
    bmi.biWidth = cw
    bmi.biHeight = -ch  # top-down
    bmi.biPlanes = 1
    bmi.biBitCount = 32
    bmi.biCompression = 0  # BI_RGB

    buf = ctypes.create_string_buffer(cw * ch * 4)
    gdi32.GetDIBits(hdc_mem, hbmp, 0, ch, buf, ctypes.byref(bmi), 0)
    gdi32.SelectObject(hdc_mem, old)
    gdi32.DeleteObject(hbmp)
    gdi32.DeleteDC(hdc_mem)
    user32.ReleaseDC(0, hdc_screen)
    user32.ReleaseDC(hwnd, hdc_window)

    # --- minimal PNG writer: BGRA -> RGB rows, filter 0 ---
    raw = bytearray()
    stride = cw * 4
    pixels = buf.raw
    for y in range(ch):
        raw.append(0)  # filter type 0
        row = pixels[y * stride:(y + 1) * stride]
        for x in range(cw):
            b = row[x * 4 + 0]
            g = row[x * 4 + 1]
            r = row[x * 4 + 2]
            raw.append(r)
            raw.append(g)
            raw.append(b)

    def chunk(tag, data):
        c = struct.pack(">I", len(data)) + tag + data
        return c + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    ihdr = struct.pack(">IIBBBBB", cw, ch, 8, 2, 0, 0, 0)
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", ihdr)
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 6))
    png += chunk(b"IEND", b"")

    with open(out_path, "wb") as f:
        f.write(png)
    return cw, ch


# ---------------------------------------------------------------- LSP client

class LspDriver:
    """Speaks LSP/JSON-RPC over slint-lsp's stdio, keeps preview window ref."""

    def __init__(self, root):
        self.root = root
        self.proc = None
        self.msg_id = 0
        self.hwnd = None
        self.reader = None
        self.diagnostics = {}
        self.lock = threading.Lock()

    def start(self):
        env = dict(os.environ)
        env["SLINT_MCP_PORT"] = str(MCP_PORT)  # preview child inherits: enables take_screenshot etc.
        env["SLINT_EMIT_DEBUG_INFO"] = "1"     # interpreter compiles .slint with debug info -> element tree/ids
        env["SLINT_BACKEND"] = "winit,run-in-thread" if sys.platform == "win32" else env.get("SLINT_BACKEND", "")
        self.proc = subprocess.Popen(
            [LSP_EXE, "-I", self.root],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL, env=env,
            creationflags=subprocess.CREATE_NO_WINDOW if hasattr(subprocess, "CREATE_NO_WINDOW") else 0,
        )
        self.reader = threading.Thread(target=self._read_loop, daemon=True)
        self.reader.start()
        self._send_request("initialize", {
            "processId": os.getpid(),
            "rootUri": "file:///" + self.root.replace("\\", "/"),
            "capabilities": {
                "window": {"showDocument": {"support": True}},
                "workspace": {"configuration": True, "didChangeConfiguration": {"dynamicRegistration": False}},
                "textDocument": {"synchronization": {"didSave": True}},
            },
            "initializationOptions": {},
            "workspaceFolders": [{"uri": "file:///" + self.root.replace("\\", "/"), "name": "root"}],
        })
        self._notify("initialized", {})
        return True

    def _read_loop(self):
        f = self.proc.stdout
        while True:
            # headers
            headers = {}
            while True:
                line = f.readline()
                if not line:
                    return
                line = line.strip()
                if not line:
                    break
                if b":" in line:
                    k, v = line.split(b":", 1)
                    headers[k.decode().strip().lower()] = v.decode().strip()
            try:
                length = int(headers.get("content-length", "0"))
            except ValueError:
                continue
            if length <= 0:
                continue
            body = f.read(length)
            try:
                msg = json.loads(body)
            except Exception:
                continue
            if "id" in msg and "result" in msg:
                continue  # response; we don't await these synchronously
            if "method" in msg:
                m = msg["method"]
                if m == "workspace/configuration":
                    self._respond(msg["id"], [{"style": None}])
                elif m == "client/registerCapability":
                    self._respond(msg["id"], None)
                elif m == "workspace/didChangeConfiguration":
                    pass
                elif m == "textDocument/publishDiagnostics":
                    # keep the latest diagnostics per file; surfaces .slint compile errors
                    params = msg.get("params", {})
                    self.diagnostics[params.get("uri", "?")] = params.get("diagnostics", [])
                    with open(os.path.join(self.root, "build", "lsp-diagnostics.json"), "w") as df:
                        json.dump(self.diagnostics, df, indent=1)

    def _respond(self, rid, result):
        self._write({"jsonrpc": "2.0", "id": rid, "result": result})

    def _write(self, obj):
        data = json.dumps(obj).encode()
        with self.lock:
            self.proc.stdin.write(b"Content-Length: %d\r\n\r\n" % len(data))
            self.proc.stdin.write(data)
            self.proc.stdin.flush()

    def _send_request(self, method, params):
        self.msg_id += 1
        self._write({"jsonrpc": "2.0", "id": self.msg_id, "method": method, "params": params})
        return self.msg_id

    def _notify(self, method, params):
        self._write({"jsonrpc": "2.0", "method": method, "params": params})

    def open_file(self, path):
        uri = "file:///" + path.replace("\\", "/")
        try:
            text = open(path, "r", encoding="utf-8").read()
        except OSError:
            return False
        self._notify("textDocument/didOpen", {
            "textDocument": {"uri": uri, "languageId": "slint", "version": 1, "text": text},
        })
        return True

    def show_preview(self, path, component):
        uri = "file:///" + path.replace("\\", "/")
        self._send_request("workspace/executeCommand", {
            "command": "slint/showPreview",
            "arguments": [uri, component],
        })
        # preview opens in a child process of the LSP; find that window
        # The LSP spawns `slint-lsp live-preview --remote-controlled`.
        # Give it time to open, then locate a window whose process is a
        # descendant of the LSP pid.
        return self._find_preview_window()

    def _descendant_pids(self, root_pid):
        """Find all PIDs whose parent chain includes root_pid (via toolhelp snapshot)."""
        th32cs_snapprocess = 0x00000002

        class PROCESSENTRY32W(ctypes.Structure):
            _fields_ = [
                ("dwSize", wt.DWORD), ("cntUsage", wt.DWORD), ("th32ProcessID", wt.DWORD),
                ("th32DefaultHeapID", ctypes.POINTER(ctypes.c_ulong)), ("th32ModuleID", wt.DWORD),
                ("cntThreads", wt.DWORD), ("th32ParentProcessID", wt.DWORD),
                ("pcPriClassBase", wt.LONG), ("dwFlags", wt.DWORD), ("szExeFile", wt.WCHAR * 260),
            ]

        snap = kernel32.CreateToolhelp32Snapshot(th32cs_snapprocess, 0)
        entry = PROCESSENTRY32W()
        entry.dwSize = ctypes.sizeof(PROCESSENTRY32W)
        parents = {}
        ok = kernel32.Process32FirstW(snap, ctypes.byref(entry))
        while ok:
            parents[entry.th32ProcessID] = entry.th32ParentProcessID
            ok = kernel32.Process32NextW(snap, ctypes.byref(entry))
        kernel32.CloseHandle(snap)

        desc = set()
        for pid, parent in parents.items():
            cur = pid
            for _ in range(10):  # bounded walk up
                if cur == root_pid:
                    desc.add(pid)
                    break
                cur = parents.get(cur, 0)
                if cur == 0:
                    break
        return desc

    def _find_preview_window(self, timeout=20.0):
        deadline = time.time() + timeout
        while time.time() < deadline:
            pids = self._descendant_pids(self.proc.pid)
            for pid in [self.proc.pid] + list(pids):
                hwnd = find_window_by_pid(pid, timeout=0.5)
                if hwnd:
                    # must look like a top-level app window (has a title, non-trivial size)
                    title = ctypes.create_unicode_buffer(256)
                    user32.GetWindowTextW(hwnd, title, 256)
                    w, h = get_client_rect(hwnd)
                    if title.value and w > 100 and h > 100:
                        self.hwnd = hwnd
                        return hwnd
            time.sleep(0.5)
        return None

    def stop(self):
        if self.proc:
            try:
                self.proc.terminate()
            except Exception:
                pass


def mcp_call(tool, args):
    """Call a tool on Slint's runtime MCP server (Streamable HTTP on MCP_PORT)."""
    body = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "tools/call",
        "params": {"name": tool, "arguments": args},
    }
    req = (
        "POST /mcp HTTP/1.1\r\n"
        f"Host: {HOST}:{MCP_PORT}\r\n"
        "Content-Type: application/json\r\n"
        "Accept: application/json, text/event-stream\r\n"
        f"Content-Length: {len(json.dumps(body))}\r\n"
        "Connection: close\r\n\r\n" + json.dumps(body)
    ).encode()
    try:
        s = socket.create_connection((HOST, MCP_PORT), timeout=30)
        s.sendall(req)
        data = b""
        while True:
            chunk = s.recv(65536)
            if not chunk:
                break
            data += chunk
        s.close()
    except OSError as e:
        return {"ok": False, "error": f"MCP connection failed: {e}"}
    # response is either application/json or an SSE stream; find the JSON body
    text = data.decode(errors="replace")
    headers, _, payload = text.partition("\r\n\r\n")
    if "text/event-stream" in headers:
        for line in payload.splitlines():
            if line.startswith("data:"):
                payload = line[5:].strip()
                break
    try:
        msg = json.loads(payload)
    except json.JSONDecodeError:
        return {"ok": False, "error": "bad MCP response", "raw": text[:300]}
    if "error" in msg:
        return {"ok": False, "error": msg["error"]}
    result = msg.get("result", {})
    if result.get("isError"):
        return {"ok": False, "error": result.get("content")}
    # unwrap single text content
    content = result.get("content")
    if isinstance(content, list) and len(content) == 1 and content[0].get("type") == "text":
        try:
            result = json.loads(content[0]["text"])
            return {"ok": True, "result": result}
        except (json.JSONDecodeError, KeyError):
            pass
    if isinstance(content, list) and len(content) == 1:
        return {"ok": True, "result": content[0]}
    return {"ok": True, "result": result}


# ---------------------------------------------------------------- control server

def run_server():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.makedirs(os.path.dirname(STATE_FILE), exist_ok=True)

    driver = LspDriver(root)
    if not driver.start():
        print("failed to start LSP", file=sys.stderr)
        return 1

    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind((HOST, PORT))
    srv.listen(8)
    print(f"live-preview server ready on {HOST}:{PORT}", flush=True)

    with open(STATE_FILE, "w") as f:
        json.dump({"port": PORT, "pid": os.getpid()}, f)

    try:
        while True:
            conn, _ = srv.accept()
            try:
                conn.settimeout(120)
                data = b""
                while b"\n" not in data:
                    chunk = conn.recv(65536)
                    if not chunk:
                        break
                    data += chunk
                if not data:
                    continue
                cmd = json.loads(data.decode())
                resp = handle(driver, cmd)
                conn.sendall(json.dumps(resp).encode() + b"\n")
            except Exception as e:
                try:
                    conn.sendall(json.dumps({"ok": False, "error": str(e)}).encode() + b"\n")
                except Exception:
                    pass
            finally:
                conn.close()
    finally:
        driver.stop()
        try:
            os.remove(STATE_FILE)
        except OSError:
            pass


def handle(driver, cmd):
    c = cmd.get("cmd")
    if c == "ping":
        return {"ok": True, "lsp_pid": driver.proc.pid}
    if c == "preview":
        path = cmd["file"]
        if not os.path.isabs(path):
            path = os.path.join(driver.root, path)
        if not os.path.exists(path):
            return {"ok": False, "error": f"no such file: {path}"}
        driver.open_file(path)
        hwnd = driver.show_preview(path, cmd["component"])
        if not hwnd:
            return {"ok": False, "error": "preview window did not open"}
        if "width" in cmd and "height" in cmd:
            resize_client(hwnd, int(cmd["width"]), int(cmd["height"]))
        time.sleep(float(cmd.get("settle", 0.6)))
        w, h = get_client_rect(hwnd)
        title = ctypes.create_unicode_buffer(256)
        user32.GetWindowTextW(hwnd, title, 256)
        return {"ok": True, "hwnd": hwnd, "size": [w, h], "title": title.value}
    if c == "resize":
        if not driver.hwnd:
            return {"ok": False, "error": "no preview window"}
        ok = resize_client(driver.hwnd, int(cmd["width"]), int(cmd["height"]))
        time.sleep(float(cmd.get("settle", 0.6)))
        return {"ok": ok, "size": get_client_rect(driver.hwnd)}
    if c == "size":
        if not driver.hwnd:
            return {"ok": False, "error": "no preview window"}
        return {"ok": True, "size": get_client_rect(driver.hwnd)}
    if c == "capture":
        if not driver.hwnd:
            return {"ok": False, "error": "no preview window"}
        out = cmd["out"]
        if not os.path.isabs(out):
            out = os.path.join(driver.root, out)
        os.makedirs(os.path.dirname(out), exist_ok=True)
        w, h = capture_window_png(driver.hwnd, out)
        return {"ok": True, "out": out, "size": [w, h]}
    if c == "mcp":
        # Raw passthrough: call any tool on Slint's MCP server.
        # e.g. {"cmd":"mcp","tool":"take_screenshot","args":{"window":{...}}}
        return mcp_call(cmd["tool"], cmd.get("args", {}))
    if c == "shot":
        # Convenience: screenshot the (first) preview window via MCP, save to out.
        wl = mcp_call("list_windows", {})
        if not wl.get("ok"):
            return wl
        res = wl["result"]
        wins = res.get("windowHandles", res.get("windows", [])) if isinstance(res, dict) else res
        if not isinstance(wins, list) or not wins:
            return {"ok": False, "error": "no windows reported by MCP"}
        win = wins[-1] if len(wins) > 1 else wins[0]
        r = mcp_call("take_screenshot", {"windowHandle": win})
        if not r.get("ok"):
            return r
        content = r["result"]
        # MCP image content block: {"type":"image","data":"<base64 png>","mimeType":"image/png"}
        if isinstance(content, dict) and content.get("type") == "image":
            out = cmd.get("out", "build/preview-mcp.png")
            if not os.path.isabs(out):
                out = os.path.join(driver.root, out)
            os.makedirs(os.path.dirname(out), exist_ok=True)
            with open(out, "wb") as f:
                f.write(base64.b64decode(content["data"]))
            return {"ok": True, "out": out, "mime": content.get("mimeType")}
        return {"ok": False, "error": "unexpected screenshot payload", "payload": str(content)[:200]}
    if c == "stop":
        threading.Thread(target=lambda: (time.sleep(0.2), os._exit(0)), daemon=True).start()
        return {"ok": True}
    return {"ok": False, "error": f"unknown command: {c}"}


def send_cmd(args):
    """Client mode: send one JSON command to the running server."""
    try:
        state = json.load(open(STATE_FILE))
    except (OSError, json.JSONDecodeError):
        print("live-preview server is not running (no state file)", file=sys.stderr)
        return 1
    cmd = {"cmd": args[0]}
    if args[0] == "preview":
        if len(args) < 3:
            print("usage: preview FILE.slint COMPONENT [WxH]", file=sys.stderr)
            return 1
        cmd["file"], cmd["component"] = args[1], args[2]
        if len(args) > 3 and "x" in args[3]:
            w, h = args[3].lower().split("x", 1)
            cmd["width"], cmd["height"] = int(w), int(h)
    elif args[0] == "capture":
        if len(args) < 2:
            print("usage: capture OUT.png", file=sys.stderr)
            return 1
        cmd["out"] = args[1]
    elif args[0] == "resize":
        if len(args) < 2 or "x" not in args[1]:
            print("usage: resize WxH", file=sys.stderr)
            return 1
        w, h = args[1].lower().split("x", 1)
        cmd["width"], cmd["height"] = int(w), int(h)
    try:
        s = socket.create_connection((HOST, state["port"]), timeout=180)
        s.sendall(json.dumps(cmd).encode() + b"\n")
        data = b""
        while b"\n" not in data:
            chunk = s.recv(65536)
            if not chunk:
                break
            data += chunk
        resp = json.loads(data.decode())
        print(json.dumps(resp, indent=2))
        return 0 if resp.get("ok") else 2
    except (ConnectionRefusedError, socket.timeout, json.JSONDecodeError) as e:
        print(f"cannot reach server: {e}", file=sys.stderr)
        return 1


def main():
    args = sys.argv[1:]
    if not args:
        return run_server()
    if args[0] == "serve":
        return run_server()
    return send_cmd(args)


if __name__ == "__main__":
    sys.exit(main() or 0)
