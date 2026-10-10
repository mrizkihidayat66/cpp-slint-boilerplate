#!/usr/bin/env python3
"""mcp-query.py — ad-hoc MCP element-tree queries for debugging.

Usage: python scripts/mcp-query.py [substring]
Prints every element of the last tracked window whose id/type name
contains the optional substring, with its absolute position and size.
"""
import json, sys, urllib.request

MCP = "http://127.0.0.1:42124/mcp"

def call(tool, args):
    body = json.dumps({"jsonrpc": "2.0", "id": 1, "method": "tools/call",
                       "params": {"name": tool, "arguments": args}}).encode()
    req = urllib.request.Request(MCP, data=body,
        headers={"Content-Type": "application/json",
                 "Accept": "application/json, text/event-stream"})
    with urllib.request.urlopen(req, timeout=60) as r:
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

def main():
    wins = json.loads(text_of(call("list_windows", {})))["windowHandles"]
    win = wins[-1]
    props = json.loads(text_of(call("get_window_properties", {"windowHandle": win})))
    root = props["rootElementHandle"]
    tree = json.loads(text_of(call("get_element_tree",
                                   {"elementHandle": root, "maxElements": 2000})))
    print("totalCount:", tree.get("totalCount"),
          "debugInfoMissing:", tree.get("debugInfoMissing"))

    want = sys.argv[1] if len(sys.argv) > 1 else ""
    for e in tree.get("elements", []):
        ids = e.get("typeNamesAndIds", [])
        t = " ".join((x.get("id") or "") + ":" + (x.get("typeName") or "")
                     for x in ids if isinstance(x, dict))
        if want and want.lower() not in t.lower():
            continue
        pos = e.get("absolutePosition") or {}
        sz = e.get("size") or {}
        print(f"{t} @({pos.get('x')},{pos.get('y')}) {sz.get('width')}x{sz.get('height')}")

if __name__ == "__main__":
    main()
