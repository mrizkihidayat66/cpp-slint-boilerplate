"""Visual screenshots of preview harnesses via the MCP live preview.

Clean-restarts the preview server per page (avoids the stale-window
pitfall), GDI-captures the single window, writes build/shot-<name>.png.

Usage: python scripts/shot-pages.py [harness component WxH name ...] (default set)
Run from the repo root; requires Pillow for the BMP->PNG conversion.
"""
import ctypes
import ctypes.wintypes as wt
import subprocess
import sys
import time
import socket
import os

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KILL = os.path.join(REPO, "scripts", "preview", "kill-lp.ps1")

user32 = ctypes.windll.user32
gdi32 = ctypes.windll.gdi32
WNDENUMPROC = ctypes.WINFUNCTYPE(wt.BOOL, wt.HWND, wt.LPARAM)


class BITMAPINFOHEADER(ctypes.Structure):
    _fields_ = [
        ("biSize", wt.DWORD), ("biWidth", wt.LONG), ("biHeight", wt.LONG),
        ("biPlanes", wt.WORD), ("biBitCount", wt.WORD),
        ("biCompression", wt.DWORD), ("biSizeImage", wt.DWORD),
        ("biXPelsPerMeter", wt.LONG), ("biYPelsPerMeter", wt.LONG),
        ("biClrUsed", wt.DWORD), ("biClrImportant", wt.DWORD),
    ]


def find_hwnds():
    hwnds = []

    @WNDENUMPROC
    def cb(hwnd, _):
        ln = user32.GetWindowTextLengthW(hwnd)
        buf = ctypes.create_unicode_buffer(ln + 1)
        user32.GetWindowTextW(hwnd, buf, ln + 1)
        if buf.value == "Slint Live-Preview" and user32.IsWindowVisible(hwnd):
            hwnds.append(hwnd)
        return True

    user32.EnumWindows(cb, 0)
    return hwnds


def capture(hwnd, out):
    class RECT(ctypes.Structure):
        _fields_ = [("l", wt.LONG), ("t", wt.LONG), ("r", wt.LONG), ("b", wt.LONG)]

    rc = RECT()
    user32.GetClientRect(hwnd, ctypes.byref(rc))
    w, h = rc.r - rc.l, rc.b - rc.t
    hdc = user32.GetDC(hwnd)
    mem = gdi32.CreateCompatibleDC(hdc)
    bmp = gdi32.CreateCompatibleBitmap(hdc, w, h)
    gdi32.SelectObject(mem, bmp)
    gdi32.BitBlt(mem, 0, 0, w, h, hdc, 0, 0, 0x00CC0020)
    bi = BITMAPINFOHEADER()
    bi.biSize = 40
    bi.biWidth, bi.biHeight = w, -h  # top-down
    bi.biPlanes, bi.biBitCount = 1, 32
    buf = (ctypes.c_ubyte * (w * h * 4))()
    gdi32.GetDIBits(mem, bmp, 0, h, buf, ctypes.byref(bi), 0)
    gdi32.DeleteObject(bmp)
    gdi32.DeleteDC(mem)
    user32.ReleaseDC(hwnd, hdc)
    rows = []
    for y in range(h):
        row = bytearray()
        base = y * w * 4
        for x in range(w):
            i = base + x * 4
            row += bytes((buf[i], buf[i + 1], buf[i + 2]))
        rows.append(bytes(row))
    pad = (4 - (w * 3) % 4) % 4
    data = b"".join(r + b"\x00" * pad for r in rows)
    hdr = BITMAPINFOHEADER()  # GetDIBits mutates the query header; write a clean 24bpp one
    hdr.biSize = 40
    hdr.biWidth, hdr.biHeight = w, -h
    hdr.biPlanes, hdr.biBitCount = 1, 24
    hdr.biSizeImage = len(data)
    with open(out, "wb") as f:
        f.write(b"BM")
        f.write((14 + 40 + len(data)).to_bytes(4, "little"))
        f.write(b"\x00" * 4)
        f.write((14 + 40).to_bytes(4, "little"))
        f.write(hdr)
        f.write(data)


def wait_port(port=42123, timeout=30):
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            with socket.create_connection(("127.0.0.1", port), timeout=1):
                return True
        except OSError:
            time.sleep(0.3)
    return False


def kill_all():
    subprocess.run(["powershell", "-ExecutionPolicy", "Bypass", "-File", KILL],
                   capture_output=True)


DEFAULT_JOBS = [
    ("scripts/preview/tasks-preview.slint", "TasksPreview", "1280x800", "tasks"),
    ("scripts/preview/form-preview.slint", "FormPreview", "1024x1000", "form"),
    ("scripts/preview/select-preview.slint", "SelectPreview", "600x500", "select"),
]


def main(jobs):
    outdir = os.path.join(REPO, "build")
    os.makedirs(outdir, exist_ok=True)
    for harness, comp, size, name in jobs:
        print(f"--- {comp} -> build/shot-{name}")
        kill_all()
        serve = subprocess.Popen([sys.executable, "scripts/live-preview.py", "serve"],
                                 stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if not wait_port():
            print("  !! port never came up")
            serve.kill()
            continue
        subprocess.run([sys.executable, "scripts/live-preview.py", "preview",
                        harness, comp, size], capture_output=True, timeout=60)
        time.sleep(2.5)
        hwnds = find_hwnds()
        if hwnds:
            bmp = os.path.join(outdir, f"shot-{name}.bmp")
            capture(hwnds[0], bmp)
            from PIL import Image
            Image.open(bmp).save(os.path.join(outdir, f"shot-{name}.png"))
            os.remove(bmp)
            print("  captured")
        serve.kill()
        kill_all()
    print("done")


if __name__ == "__main__":
    jobs = DEFAULT_JOBS
    if len(sys.argv) >= 5 and (len(sys.argv) - 1) % 4 == 0:
        jobs = [tuple(sys.argv[i:i + 4]) for i in range(1, len(sys.argv), 4)]
    main(jobs)
