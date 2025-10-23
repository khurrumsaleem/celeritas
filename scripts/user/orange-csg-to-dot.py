#!/usr/bin/env python3
# Copyright Celeritas contributors: see top-level COPYRIGHT file for details
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
"""
Convert an ORANGE CSG JSON representation to a GraphViz or Mermaid input.

.. example::

    To run from a raw CSG tree that you've copied to your clipboard::

        pbpaste | ./orange-csg-to-dot.py -T dot - | dot -Tpdf -o tree.pdf

    To copy a mermaid plot generated from universe 2 of an external .csg.json
    ORANGE debug output, so that you can paste on github::

       ./orange-csg-to-dot.py -T mermaid -u 2 | pbcopy

"""

from itertools import count, repeat
from contextlib import contextmanager
import json
import sys
from typing import Any


class DotGenerator:
    def __init__(self, f, args):
        self.f = f
        self.write = f.write
        self.vol_edges = []

    def __enter__(self):
        self.write("""\
strict digraph {
rankdir=TB
node [shape=box]
""")
        return self

    def __exit__(self, type, value, traceback):
        self.write("""\
subgraph volume_edges {
edge [color=gray, dir=both]
""")
        for i in self.vol_edges:
            self.write(f"volume{i:02d} -> {i:02d}\n")
        self.write("}\n}\n")

    def write_node(self, i: int, label: str):
        label = f' [label="{label}"]' if label else ""
        self.write(f"{i:02d}{label}\n")

    @contextmanager
    def write_volumes(self):
        self.write("""\
subgraph volumes {
rank = same
cluster=true
label = \"Volumes\"
node [style=rounded, shape=box]
""")
        yield self.write_volume
        self.write("}\n")

    def write_volume(self, i: int, label: str):
        label = f' [label="{label}"]' if label else ""
        self.write(f"volume{i:02d}{label}\n")
        self.vol_edges.append(i)

    def write_edge(self, i: int, e: int):
        self.write(f"{i:02d} -> {e:02d};\n")


class MermaidGenerator:
    def __init__(self, f, args):
        self.f = f
        self.write = f.write
        self.vol_edges = []

    def __enter__(self):
        self.write("flowchart TB\n")
        return self

    def __exit__(self, type, value, traceback):
        for i in self.vol_edges:
            self.write(f"  v{i:02d} <--> n{i:02d}\n")

    def write_node(self, i: int, label: str):
        label = f'["{label}"]' if label else ""
        self.write(f"  n{i:02d}{label}\n")

    @contextmanager
    def write_volumes(self):
        self.write("""\
subgraph Volumes
""")
        yield self.write_volume
        self.write("end\n")

    def write_volume(self, i: int, label: str):
        label = f'(["{label}"])' if label else ""
        self.write(f"  v{i:02d}{label}\n")
        self.vol_edges.append(i)

    def write_edge(self, i: int, e: int):
        self.write(f"  n{i:02d} --> n{e:02d}\n")


def write_tree(gen: DotGenerator, csg_unit: dict[str, Any], args):
    tree = csg_unit["tree"]
    metadata = csg_unit["metadata"] or repeat(None)
    get_surface = (
        csg_unit["surfaces"].__getitem__ if "surfaces" in csg_unit else (lambda i: None)
    )
    print_surf = args.print_surface
    print_id = args.print_id

    for i, node, labels in zip(count(), tree, metadata):
        if isinstance(node, str):
            # True literal
            node = (node.upper(), node)

        (nodetype, value) = node

        if nodetype == "S":
            if print_surf and (s := get_surface(value)):
                labels.append(s)
            if print_id:
                labels.append(f"[N{i:02d} = S{value}]")
        else:
            if print_id:
                labels.append(f"[N{i:02d}]")
            labels.append(nodetype)

        label = r"\n".join(labels)
        gen.write_node(i, label)

        if isinstance(value, list):
            # Joined
            for v in value:
                gen.write_edge(i, v)
        elif nodetype in "=~":
            # Aliased/negated
            gen.write_edge(i, value)


def write_volumes(gen, volumes: dict[str, Any]):
    if not volumes:
        return

    with gen.write_volumes() as write_volume:
        for v in volumes:
            write_volume(v["csg_node"], v.get("label"))


def run(infile, outfile, gencls, args):
    tree = json.load(infile)
    universe = args.universe
    if universe is not None:
        # Load from a .csg.json debug file
        csg_unit = tree[universe]
    elif isinstance(tree, list) and isinstance(tree[0], dict) and "tree" in tree[0]:
        # No -u option given, but it's a csg.json file
        num_univ = len(tree)
        if num_univ == 1:
            print(
                "Input tree is a CSG listing with one unit:assuming `-u N=0`",
                file=sys.stderr,
            )
            csg_unit = tree[0]
        else:
            print(
                "Input tree is a CSG listing: please rerun with -u N "
                f"where 0 ≤ N < {num_univ}",
                file=sys.stderr,
            )
            sys.exit(1)
    else:
        # Raw CSG tree, no metadata
        csg_unit = {
            "tree": tree,
            "metadata": None,
            "label": "CSG tree",
        }

    with gencls(outfile, args) as gen:
        write_tree(gen, csg_unit, args)
        if vols := csg_unit.get("volumes"):
            write_volumes(gen, vols)


def main():
    import argparse

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", help="Input filename (- for stdin)")
    parser.add_argument(
        "-T", "--type", default=None, help="Output type: 'dot' or 'mermaid'"
    )
    parser.add_argument(
        "-u",
        "--universe",
        type=int,
        default=None,
        help="Universe ID if a 'csg.json' debug file",
    )
    parser.add_argument(
        "--print-id", action="store_true", help="print CsgTree node ids"
    )
    parser.add_argument(
        "--print-surface", action="store_true", help="print surface definitions"
    )
    parser.add_argument(
        "-o", "--output", default=None, help="Output filename (empty for stdout)"
    )
    args = parser.parse_args()

    if args.input == "-":
        infile = sys.stdin
    else:
        infile = open(args.input)

    if not args.type:
        if args.output and args.output.endswith(".dot"):
            gencls = DotGenerator
        else:
            gencls = MermaidGenerator
    else:
        gencls_dict = {
            "dot": DotGenerator,
            "mermaid": MermaidGenerator,
        }
        try:
            gencls = gencls_dict[args.type]
        except KeyError:
            valid = ",".join(gencls_dict)
            print(f"invalid type {args.type}: valid types are {valid}", file=sys.stderr)
            sys.exit(1)

    if not args.output:
        outfile = sys.stdout
    else:
        outfile = open(args.output, "w")

    run(infile, outfile, gencls, args)


if __name__ == "__main__":
    main()
