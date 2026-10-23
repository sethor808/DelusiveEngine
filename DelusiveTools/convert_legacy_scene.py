#!/usr/bin/env python3
"""Convert pre-flat (nested) .scene and .agent files to the flat UUID block format.

Old format nests [Agent]/[Component]/[System] blocks inside [Scene] with closing
tags, and ScriptComponent params inside { } braces. The flat format gives every
block a UUID in its header and parents list their children by UUID:

    [Scene <id>]            agents=<id> <id>   systems=<id>
    [Agent <Type> <id>]     components=<id> <id>
    [Component <Type> <id>]
    [BehaviourScript <Type> <id>]   (referenced from ScriptComponent's script=)
    [System <Type> <id>]

Existing id= values are kept so cross references (e.g. a script's target) still
resolve. Blocks without one get a fresh UUID.

The oldest files use "key value" instead of "key=value"; both are accepted.

Usage: convert_legacy_scene.py [--dry-run] <file.scene|file.agent>...
"""
import os
import sys

#Registered std::string properties - the loader reads unquoted strings only up to
#the first space, so these get quoted
STRING_KEYS = {"name", "activeCanvasName"}

#Script params that were renamed in the flat format
SCRIPT_KEY_RENAMES = {"targetID": "target"}


def new_uuid():
    return os.urandom(8).hex() + "-" + os.urandom(8).hex()


class Block:
    def __init__(self, category, type_=""):
        self.category = category
        self.type = type_
        self.id = None
        self.props = {}
        self.children = []


def fix_value(key, value):
    if key in STRING_KEYS and not value.startswith('"'):
        return '"' + value.replace('\\', '\\\\').replace('"', '\\"') + '"'
    if key == "textureData":
        #Backslashes only work as separators on Windows; forward slashes work everywhere
        return value.replace('\\', '/')
    return value


def parse_legacy(lines):
    """Return the root Scene/Agent Block, or None if the file is not in the legacy format."""
    root = None
    stack = []
    script_params = None  #dict while inside a ScriptComponent's { } section

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
                script_params[SCRIPT_KEY_RENAMES.get(key, key)] = value
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
                return None  #header already carries a UUID - flat format
            block = Block(parts[0], parts[1] if len(parts) > 1 else "")
            if stack:
                stack[-1].children.append(block)
            elif block.category in ("Scene", "Agent"):
                root = block
            stack.append(block)
            continue

        if not stack:
            continue

        if '=' in line:
            key, value = (s.strip() for s in line.split('=', 1))
        else:
            key, _, value = line.partition(' ')
            value = value.strip()
            #Old scene header lines "agents <count>" / "systems <count>" are rebuilt as id lists
            if stack[-1].category == "Scene" and key != "name":
                continue

        if key == "id":
            stack[-1].id = value
        stack[-1].props[key] = fix_value(key, value)

    return root


def flatten_agent(agent, out):
    agent.id = agent.id or new_uuid()
    components = [c for c in agent.children if c.category == "Component"]
    for comp in components:
        comp.id = comp.id or new_uuid()
    agent.props["components"] = " ".join(c.id for c in components)
    agent.props["id"] = agent.id
    out.append(agent)

    for comp in components:
        comp.props["id"] = comp.id
        out.append(comp)

        #Old ScriptComponents stored the script type inline; now script= points at its own block
        if comp.type == "ScriptComponent" and "script" in comp.props:
            script = Block("BehaviourScript", comp.props["script"])
            script.id = new_uuid()
            script.props = dict(getattr(comp, "script_params", {}))
            script.props["id"] = script.id
            comp.props["script"] = script.id
            out.append(script)


def flatten(root):
    out = []

    if root.category == "Agent":
        flatten_agent(root, out)
        return out

    scene = root
    scene.id = scene.id or new_uuid()
    agents = [c for c in scene.children if c.category == "Agent"]
    systems = [c for c in scene.children if c.category == "System"]

    for block in agents + systems:
        block.id = block.id or new_uuid()

    scene.props["agents"] = " ".join(a.id for a in agents)
    scene.props["systems"] = " ".join(s.id for s in systems)
    out.append(scene)

    for agent in agents:
        flatten_agent(agent, out)

    for sys_block in systems:
        sys_block.props["id"] = sys_block.id
        out.append(sys_block)

    return out


def write_flat(blocks):
    #Mirrors DelusiveParser::WriteBlock - sorted keys, blank line between blocks
    text = []
    for block in blocks:
        header = block.category
        if block.type:
            header += " " + block.type
        header += " " + block.id
        text.append("[" + header + "]")
        for key in sorted(block.props):
            text.append(key + "=" + block.props[key])
        text.append("")
    return "\n".join(text)


def main(argv):
    dry_run = "--dry-run" in argv
    paths = [a for a in argv if a != "--dry-run"]
    if not paths:
        print(__doc__.strip().splitlines()[-1])
        return 1

    for path in paths:
        with open(path, encoding="utf-8") as f:
            scene = parse_legacy(f.read().splitlines())

        if scene is None:
            print(f"skip  {path} (already flat or no top-level [Scene]/[Agent] block)")
            continue

        result = write_flat(flatten(scene))

        if dry_run:
            print(f"==== {path}\n{result}")
            continue

        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(result)
        print(f"wrote {path}")

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
