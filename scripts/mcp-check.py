#!/usr/bin/env python3
"""Check the MCP element tree of every tracked window."""
import json
import urllib.request

MCP = "http://127.0.0.1:42124/mcp"


def call(tool, args):
    body = json.dumps(
        {"jsonrpc": "2.0", "id": 1, "method": "tools/call",
         "params": {"name": tool, "arguments": args}}
    ).encode()
    req = urllib.request.Request(
        MCP, data=body,
        headers={"Content-Type": "application/json",
                 "Accept": "application/json, text/event-stream"})
    with urllib.request.urlopen(req, timeout=30) as r:
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
    print("windows:", wins)
    for win in wins:
        props = json.loads(text_of(call("get_window_properties", {"windowHandle": win})))
        root = props["rootElementHandle"]
        tree = json.loads(text_of(
            call("get_element_tree", {"elementHandle": root, "maxElements": 500})))
        print(f"win {win['index']}: totalCount={tree.get('totalCount')} "
              f"debugInfoMissing={tree.get('debugInfoMissing')}")
        for e in tree.get("elements", [])[:12]:
            print("  ", e.get("typeNamesAndIds"), e.get("size"))


if __name__ == "__main__":
    main()
