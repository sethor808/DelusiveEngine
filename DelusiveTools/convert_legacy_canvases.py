#!/usr/bin/env python3
"""Convert the legacy shared canvas file into one flat UUID .canvas file per canvas.

Old format (ui_canvases.txt): a canvas count line, then nested [UICanvas] blocks
holding [UIElement <Type>] blocks with closing tags. UIScriptContainers store their
script type inline as scriptContainer=<Type> with the script's params in { } braces.

New format, one file per canvas:

    [UICanvas <id>]               name, elements=<id> <id>
    [UIElement <Type> <id>]       children=<id> <id>   (repeat containers rebuild theirs)
    [UIScript <Type> <id>]        referenced from a UIScriptContainer's script=

Element ids are kept, so links between elements (EquipScreen -> its containers) still
resolve. Scene files given with --scenes have their UIManager blocks rewritten from
canvas names (activeCanvasName, canvasList) to canvas ids (activeCanvas, canvases).

Usage: convert_legacy_canvases.py [--dry-run] <ui_canvases.txt> <out dir> [--scenes <file.scene>...]
"""
import os
import re
import shlex
import sys

from convert_legacy_scene import Block, fix_value, new_uuid, write_flat

#UIElement types whose children are generated at runtime and never saved
GENERATED_CHILDREN = {"UIRepeatContainer"}


def parse_canvases(lines):
    canvases = []
    stack = []
    script_params = None

    for raw in lines:
        line = raw.strip()
        if not line or line.startswith('#'):
            continue

        if script_params is not None:
            if line == '}':
                stack[-1].script_params = script_params
                script_params = None
            elif '=' in line:
                key, value = (s.strip() for s in line.split('=', 1))
                script_params[key] = value
            continue

        if line == '{':
            script_params = {}
            continue

        if line.startswith('[/'):
            stack.pop()
            continue

        if line.startswith('['):
            parts = line[1:line.rindex(']')].split()
            if len(parts) > 2:
                raise SystemExit("header already carries a UUID - file is not in the legacy format")
            block = Block(parts[0], parts[1] if len(parts) > 1 else "")
            if stack:
                stack[-1].children.append(block)
            elif block.category == "UICanvas":
                canvases.append(block)
            stack.append(block)
            continue

        #Lines before the first block (the canvas count) carry nothing
        if not stack or '=' not in line:
            continue

        key, value = (s.strip() for s in line.split('=', 1))
        if key == "id":
            stack[-1].id = value
        stack[-1].props[key] = fix_value(key, value)

    return canvases


def flatten_element(element, out):
    element.id = element.id or new_uuid()
    element.props["id"] = element.id
    out.append(element)

    #Old containers named their script type inline; now script= points at its own block
    if "scriptContainer" in element.props:
        script = Block("UIScript", element.props.pop("scriptContainer"))
        script.id = new_uuid()
        script.props = dict(getattr(element, "script_params", {}))
        script.props["id"] = script.id
        element.props["script"] = script.id
        out.append(script)

    children = [c for c in element.children if c.category == "UIElement"]
    if element.type in GENERATED_CHILDREN:
        return

    for child in children:
        child.id = child.id or new_uuid()
    element.props["children"] = " ".join(c.id for c in children)
    for child in children:
        flatten_element(child, out)


def flatten_canvas(canvas):
    out = []
    canvas.id = new_uuid()
    canvas.props["id"] = canvas.id

    elements = [c for c in canvas.children if c.category == "UIElement"]
    for element in elements:
        element.id = element.id or new_uuid()
    canvas.props["elements"] = " ".join(e.id for e in elements)
    out.append(canvas)

    for element in elements:
        flatten_element(element, out)
    return out


def canvas_name(canvas):
    return shlex.split(canvas.props.get("name", '"Unnamed"'))[0]


def migrate_scene(text, ids_by_name):
    """Rewrites UIManager blocks from canvas names to canvas ids; returns (text, changed)."""
    header = re.compile(r'^\[System UIManager [0-9a-f]{16}-[0-9a-f]{16}\]$', re.M)
    out = []
    changed = False
    pos = 0

    for match in header.finditer(text):
        start = match.end() + 1
        end = text.find('\n[', start)
        end = len(text) if end == -1 else end + 1
        out.append(text[pos:start])

        props = {}
        for line in text[start:end].splitlines():
            if '=' in line:
                key, value = line.split('=', 1)
                props[key] = value

        if "activeCanvasName" in props or "canvasList" in props:
            changed = True
            active = shlex.split(props.pop("activeCanvasName", '""') or '""')
            active_id = ids_by_name.get(active[0], "") if active else ""
            props["activeCanvas"] = active_id

            listed = shlex.split(props.pop("canvasList", "0"))
            names = listed[1:]
            ids = [ids_by_name[n] for n in names if n in ids_by_name]
            for missing in (n for n in names if n not in ids_by_name):
                print(f"warning: scene references unknown canvas {missing!r}")
            #canvases is a string list: count then quoted ids
            props["canvases"] = " ".join([str(len(ids))] + [f'"{i}"' for i in ids])

        body = "".join(f"{k}={props[k]}\n" for k in sorted(props))
        out.append(body + ("\n" if end < len(text) else ""))
        pos = end

    out.append(text[pos:])
    return "".join(out), changed


def main(argv):
    dry_run = "--dry-run" in argv
    argv = [a for a in argv if a != "--dry-run"]
    scenes = []
    if "--scenes" in argv:
        at = argv.index("--scenes")
        scenes, argv = argv[at + 1:], argv[:at]
    if len(argv) != 2:
        print(__doc__.strip().splitlines()[-1])
        return 1

    source, out_dir = argv
    with open(source, encoding="utf-8") as f:
        canvases = parse_canvases(f.read().splitlines())

    ids_by_name = {}
    for canvas in canvases:
        name = canvas_name(canvas)
        blocks = flatten_canvas(canvas)
        ids_by_name[name] = canvas.id
        path = os.path.join(out_dir, name + ".canvas")

        if dry_run:
            print(f"==== {path}\n{write_flat(blocks)}")
            continue

        if os.path.exists(path):
            print(f"skip  {path} (already exists)")
            continue
        os.makedirs(out_dir, exist_ok=True)
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(write_flat(blocks))
        print(f"wrote {path} ({len(blocks)} blocks)")

    for scene in scenes:
        with open(scene, encoding="utf-8") as f:
            text, changed = migrate_scene(f.read(), ids_by_name)
        if not changed:
            continue
        if dry_run:
            print(f"==== {scene} (UIManager)\n{text}")
            continue
        with open(scene, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        print(f"updated {scene}")

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
