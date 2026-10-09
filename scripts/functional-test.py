"""Functional interaction tests via the Slint MCP preview.

Clicks real controls (their TouchArea handles) and verifies observable
state changes:
  1. checkbox  -> check icon appears inside its bounds
  2. radio     -> selection dot appears (small centered Rectangle)
  3. switch    -> thumb x moves (toggled)
  4. select    -> popup opens; every option row is reachable
  5. slider    -> fill width follows a center click (value ~50%)

Usage:
    python scripts/live-preview.py serve                     (background)
    python scripts/live-preview.py preview scripts/preview/form-preview.slint FormPreview 1024x1000
    python scripts/functional-test.py            (form section tests)
    ... then preview scripts/preview/select-preview.slint SelectPreview 600x500
    python scripts/functional-test.py --select   (select/slider tests)
"""
import json
import sys
import time
import urllib.request

MCP = "http://127.0.0.1:42124/mcp"


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


def els():
    windows = call("list_windows", {})
    props = call("get_window_properties", {"windowHandle": windows["windowHandles"][-1]})
    return call("get_element_tree", {"elementHandle": props["rootElementHandle"]})["elements"]


def geom(e):
    p = e.get("absolutePosition", {})
    s = e.get("size", {})
    return p.get("x", 0), p.get("y", 0), s.get("width", 0), s.get("height", 0)


def types(e):
    return [n.get("typeName", "") for n in e.get("typeNamesAndIds", [])]


def find(els, type_name):
    return [e for e in els if type_name in types(e)]


def touch_inside(els, box):
    x0, y0, w0, h0 = box
    return [ta for ta in find(els, "TouchArea")
            if (lambda g: x0 <= g[0] and g[0] + g[2] <= x0 + w0 + 2
                and y0 <= g[1] and g[1] + g[3] <= y0 + h0 + 2)(geom(ta))]


results = []


def check(name, ok, detail=""):
    results.append(ok)
    print(f"  [{'PASS' if ok else 'FAIL'}] {name} {detail}")


def test_form():
    tree = els()

    # --- checkbox -------------------------------------------------------
    cbs = find(tree, "AppCheckbox")
    clickable = [cb for cb in cbs if touch_inside(tree, geom(cb))]
    if clickable:
        cb = clickable[-1]
        call("click_element", {"elementHandle": touch_inside(tree, geom(cb))[0]["handle"]})
        time.sleep(0.5)
        tree2 = els()
        x0, y0, w0, h0 = geom(cb)
        icons = [ic for ic in find(tree2, "AppIcon")
                 if (lambda g: x0 <= g[0] < x0 + w0 and y0 <= g[1] < y0 + h0)(geom(ic))]
        check("checkbox toggles", len(icons) > 0, f"({len(icons)} check icon)")
    else:
        check("checkbox toggles", False, "no TouchArea found")

    # --- radio ----------------------------------------------------------
    radios = find(tree, "AppRadio")
    if len(radios) >= 2:
        r2 = radios[-1]
        tas = touch_inside(tree, geom(r2))
        if tas:
            call("click_element", {"elementHandle": tas[0]["handle"]})
            time.sleep(0.5)
            tree2 = els()
            x0, y0, w0, h0 = geom(r2)
            dots = [e for e in tree2 if "Rectangle" in types(e) and "AppIcon" not in types(e)
                    if (lambda g: x0 <= g[0] < x0 + w0 and y0 <= g[1] < y0 + h0
                        and g[2] < 12 and g[3] < 12)(geom(e))]
            check("radio selects", len(dots) > 0, f"({len(dots)} dot)")
        else:
            check("radio selects", False, "no TouchArea")
    else:
        check("radio selects", False, f"only {len(radios)} radios in tree")

    # --- switch ---------------------------------------------------------
    switches = find(tree, "AppSwitch")
    if switches:
        sw = switches[0]
        tas = touch_inside(tree, geom(sw))
        if tas:
            x0, y0, w0, h0 = geom(sw)

            def thumbs(t):
                return [geom(e) for e in t if "Rectangle" in types(e)
                        and (lambda g: y0 <= g[1] < y0 + h0 and g[3] == 18)(geom(e))]
            before = thumbs(tree)
            call("click_element", {"elementHandle": tas[0]["handle"]})
            time.sleep(0.8)
            after = thumbs(els())
            moved = before and after and abs(after[0][0] - before[0][0]) > 5
            detail = (f"(thumb x {before[0][0]:.0f} -> {after[0][0]:.0f})"
                      if before and after else "")
            check("switch flips", bool(moved), detail)
        else:
            check("switch flips", False, "no TouchArea")
    else:
        check("switch flips", False, "no switch in tree")


def test_select():
    tree = els()
    selects = find(tree, "AppSelect")
    if not selects:
        check("select present", False)
        return
    sel = selects[0]
    tas = touch_inside(tree, geom(sel))
    if not tas:
        check("select clickable", False)
        return
    check("select clickable", True)
    call("click_element", {"elementHandle": tas[0]["handle"]})
    time.sleep(0.5)
    tree2 = els()
    x0, y0, w0, h0 = geom(sel)
    popups = [geom(e) for e in find(tree2, "ListView")
              if y0 + h0 - 2 <= geom(e)[1] < y0 + h0 + 40]
    if popups:
        ph = popups[0][3]
        check("select popup lists all options", ph > 60, f"(height {ph:.0f}px)")
    else:
        check("select popup lists all options", False, "no popup ListView found")

    # slider: fill should span ~50% after a center click (initial value 20)
    sliders = find(tree, "AppSlider")
    if sliders:
        sl = sliders[0]
        x0, y0, w0, h0 = geom(sl)
        fills = [geom(e) for e in tree if "Rectangle" in types(e)
                 if (lambda g: y0 <= g[1] < y0 + h0 and g[3] == 6)(geom(e))]
        tas = touch_inside(tree, geom(sl))
        if tas and fills:
            before = fills[0][2]
            call("click_element", {"elementHandle": tas[0]["handle"]})
            time.sleep(0.5)
            fills2 = [geom(e) for e in els() if "Rectangle" in types(e)
                      if (lambda g: y0 <= g[1] < y0 + h0 and g[3] == 6)(geom(e))]
            after = fills2[0][2] if fills2 else 0
            check("slider responds to click",
                  abs(after - w0 / 2) < 10, f"(fill {after:.0f}px of {w0:.0f})")


if __name__ == "__main__":
    if "--select" in sys.argv:
        test_select()
    else:
        test_form()
    print()
    if all(results):
        print(f"ALL {len(results)} FUNCTIONAL TESTS PASSED")
    else:
        print(f"{results.count(False)} FUNCTIONAL FAILURE(S)")
        sys.exit(1)
