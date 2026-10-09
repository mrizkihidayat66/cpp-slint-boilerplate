"""Numerical alignment verification via the Slint MCP live preview.

Dumps element geometry (absolutePosition + size + typeNames) and checks
icon-vs-text vertical centering: every AppIcon sharing a row with a Text
must have its vertical center within TOL of that Text block's center.
Exit code 1 on any failure, with measured deltas printed.

Usage:
    python scripts/live-preview.py serve          (background)
    python scripts/live-preview.py preview <harness> <Component> <WxH>
    python scripts/verify-align.py
"""
import json
import sys
import urllib.request

MCP = "http://127.0.0.1:42124/mcp"
TOL = 2.0  # px tolerance for center alignment


def call(name, args):
    req = urllib.request.Request(
        MCP,
        data=json.dumps({"jsonrpc": "2.0", "id": 1, "method": "tools/call",
                         "params": {"name": name, "arguments": args}}).encode(),
        headers={"Content-Type": "application/json",
                 "Accept": "application/json, text/event-stream"})
    with urllib.request.urlopen(req, timeout=30) as resp:
        body = json.loads(resp.read().decode())
    return json.loads(body["result"]["content"][0]["text"])


def elements():
    windows = call("list_windows", {})
    handle = windows["windowHandles"][-1]
    props = call("get_window_properties", {"windowHandle": handle})
    return call("get_element_tree", {"elementHandle": props["rootElementHandle"]})["elements"]


def pos(e):
    p = e.get("absolutePosition", {})
    return p.get("x", 0.0), p.get("y", 0.0)


def size(e):
    s = e.get("size", {})
    return s.get("width", 0.0), s.get("height", 0.0)


def types(e):
    return [t.get("typeName", "") for t in e.get("typeNamesAndIds", [])]


def run():
    els = elements()
    icons = [e for e in els if "AppIcon" in types(e)]
    texts = [e for e in els if "Text" in types(e)]
    print(f"elements={len(els)} icons={len(icons)} texts={len(texts)}")

    failures = []
    matched = 0
    for ic in icons:
        ix, iy = pos(ic)
        iw, ih = size(ic)
        ic_cy = iy + ih / 2
        # Texts to the right, overlapping the icon row; merge vertically
        # ADJACENT texts (gap < 4px, e.g. title+subtitle) into one band.
        seeds = []
        for tx in texts:
            txx, tyy = pos(tx)
            tw, th = size(tx)
            if tyy < iy + ih and iy < tyy + th and 0 < txx - ix < 300:
                seeds.append([tyy, tyy + th])
        if not seeds:
            continue
        seeds.sort()
        band = list(seeds[0])
        for s in seeds[1:]:
            if s[0] - band[1] < 4:
                band[1] = max(band[1], s[1])
        band_cy = (band[0] + band[1]) / 2
        d = abs(band_cy - ic_cy)
        matched += 1
        status = "OK" if d <= TOL else "FAIL"
        print(f"  [{status}] icon@({ix:.1f},{iy:.1f} {iw:.0f}x{ih:.0f}) "
              f"text-band y {band[0]:.1f}..{band[1]:.1f} dy={d:.2f}px")
        if d > TOL:
            failures.append(f"icon@({ix:.0f},{iy:.0f}) misaligned by {d:.2f}px")

    print(f"matched pairs: {matched}/{len(icons)}")
    return failures


if __name__ == "__main__":
    fails = run()
    print()
    if fails:
        print(f"{len(fails)} ALIGNMENT FAILURE(S):")
        for f in fails:
            print(" -", f)
        sys.exit(1)
    print("ALL ICON/TEXT ALIGNMENT CHECKS PASSED")
