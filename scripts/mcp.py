#!/usr/bin/env python3
"""mcp.py — reusable helpers for the Slint LSP MCP server (port 42124)."""
import json, urllib.request

MCP = "http://127.0.0.1:42124/mcp"

def call(tool, args, timeout=120):
    body = json.dumps({"jsonrpc": "2.0", "id": 1, "method": "tools/call",
                       "params": {"name": tool, "arguments": args}}).encode()
    req = urllib.request.Request(MCP, data=body,
        headers={"Content-Type": "application/json",
                 "Accept": "application/json, text/event-stream"})
    with urllib.request.urlopen(req, timeout=timeout) as r:
        data = r.read().decode()
    if data.lstrip().startswith(("event:", "data:")):
        for line in data.splitlines():
            if line.startswith("data:"):
                return json.loads(line[5:])
    return json.loads(data)

def text_of(r):
    res = r.get("result", r)
    try:
        return res["content"][0]["text"]
    except Exception:
        return json.dumps(res)

def windows():
    return json.loads(text_of(call("list_windows", {})))["windowHandles"]

def window_props(win):
    return json.loads(text_of(call("get_window_properties", {"windowHandle": win})))

def last_window():
    return windows()[-1]

def element_tree(win=None, max_elements=2000):
    win = win or last_window()
    props = window_props(win)
    return json.loads(text_of(call("get_element_tree",
        {"elementHandle": props["rootElementHandle"], "maxElements": max_elements})))

def find_by_id(qid):
    return json.loads(text_of(call("find_elements_by_id", {"id": qid})))

def element_props(handle):
    return json.loads(text_of(call("get_element_properties", {"elementHandle": handle})))

def query_descendants(handle, query):
    return json.loads(text_of(call("query_element_descendants",
        {"elementHandle": handle, "queryStack": query})))

def geom(e):
    """(x, y, w, h) from an element-tree entry."""
    p = e.get("absolutePosition") or {}
    s = e.get("size") or {}
    return p.get("x"), p.get("y"), s.get("width"), s.get("height")

def tree_named(tree, name):
    """All elements whose typeNamesAndIds mention `name`."""
    out = []
    for e in tree.get("elements", []):
        t = " ".join((x.get("id") or "?") + ":" + (x.get("typeName") or "?")
                     for x in e.get("typeNamesAndIds", []))
        if name in t:
            out.append((t, geom(e), e.get("handle")))
    return out

def click(handle, action="click", button="left"):
    return call("click_element", {"elementHandle": handle,
                                  "action": action, "button": button})

def scroll(handle, dx=0.0, dy=0.0):
    return call("scroll_element", {"elementHandle": handle,
                                   "deltaX": dx, "deltaY": dy})

def set_value(handle, value):
    return call("set_element_value", {"elementHandle": handle, "value": value})
