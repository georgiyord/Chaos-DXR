import argparse
import re
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("input", type=Path)
parser.add_argument("output", type=Path)
parser.add_argument("varName", type=str)
args = parser.parse_args()

data = args.input.read_bytes()

with open(args.output, "w") as file:
    file.write("#pragma once\n")
    file.write("const unsigned char " + args.varName + "[] = {")
    for byte in data:
        file.write(hex(byte))
        file.write(", ")
    file.write("};")