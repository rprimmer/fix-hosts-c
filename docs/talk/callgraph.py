"""Write fix-hostfiles' call graph as Graphviz, from clang's JSON AST.

Usage: python3 callgraph.py [--files] SOURCE.c ...

Only calls between functions defined in the given sources are kept. With
--files, functions are folded into their source file and each edge is
labeled with the number of distinct caller-callee pairs. With --table, a
Typst-readable CSV of functions per file is written instead.
"""

import collections
import json
import os
import subprocess
import sys

CLANG = ["clang", "-D_POSIX_C_SOURCE=200809L", "-std=c11", "-fsyntax-only",
         "-Xclang", "-ast-dump=json"]


def ast(path):
    out = subprocess.run(CLANG + [path], check=True, capture_output=True).stdout
    return json.loads(out)


def walk(node, state, visit):
    """Visit nodes in source order, tracking clang's delta-encoded file name."""
    for key in ("loc", "range"):
        value = node.get(key)
        if isinstance(value, dict):
            for loc in (value, value.get("begin"), value.get("expansionLoc")):
                if isinstance(loc, dict) and "file" in loc:
                    state["file"] = loc["file"]
    visit(node, state)
    for child in node.get("inner", []):
        walk(child, state, visit)


def collect(sources):
    defined = {}                      # function -> file
    calls = collections.defaultdict(set)
    wanted = {os.path.basename(s) for s in sources}

    for source in sources:
        state = {"file": source, "func": None}

        def visit(node, state):
            kind = node.get("kind")
            if kind == "FunctionDecl" and any(
                    c.get("kind") == "CompoundStmt" for c in node.get("inner", [])):
                name = os.path.basename(state["file"])
                state["func"] = node["name"] if name in wanted else None
                if state["func"]:
                    defined[node["name"]] = name
            elif kind == "DeclRefExpr" and state["func"]:
                ref = node.get("referencedDecl", {})
                if ref.get("kind") == "FunctionDecl":
                    calls[state["func"]].add(ref["name"])

        walk(ast(source), state, visit)

    edges = sorted((a, b) for a, bs in calls.items() for b in bs if b in defined)
    return defined, edges


def functions_dot(defined, edges):
    print("digraph calls {")
    print('  graph [rankdir=LR, fontname="Helvetica", fontsize=11, nodesep=0.15, ranksep=0.5];')
    print('  node [shape=box, style="rounded", fontname="Menlo", fontsize=10, height=0.25];')
    print('  edge [color="#555555", arrowsize=0.6];')
    by_file = collections.defaultdict(list)
    for func, name in defined.items():
        by_file[name].append(func)
    for i, (name, funcs) in enumerate(sorted(by_file.items())):
        print(f'  subgraph cluster_{i} {{ label="{name}"; style="rounded"; color="#bbbbbb";')
        for func in sorted(funcs):
            print(f'    "{func}";')
        print("  }")
    for a, b in edges:
        print(f'  "{a}" -> "{b}";')
    print("}")


def files_dot(defined, edges):
    pairs = collections.Counter(
        (defined[a], defined[b]) for a, b in edges if defined[a] != defined[b])
    print("digraph files {")
    print('  graph [rankdir=LR, fontname="Helvetica"];')
    print('  node [shape=box, style="rounded", fontname="Menlo", fontsize=12];')
    print('  edge [fontname="Helvetica", fontsize=10, color="#555555"];')
    for (a, b), n in sorted(pairs.items()):
        print(f'  "{a}" -> "{b}" [label="{n}"];')
    print("}")


def table(defined, edges):
    counts = collections.Counter(defined.values())
    print("file,functions")
    for name, n in counts.most_common():
        print(f"{name},{n}")
    print(f"total,{len(defined)}")
    print(f"calls,{len(edges)}")


def main(argv):
    mode = functions_dot
    if argv and argv[0] in ("--files", "--table"):
        mode = files_dot if argv.pop(0) == "--files" else table
    mode(*collect(argv))


if __name__ == "__main__":
    main(sys.argv[1:])
