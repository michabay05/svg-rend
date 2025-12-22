import sys
from svgpathtools import svg2paths

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <INPUT_SVG>")
    sys.exit(1)

paths, attributes = svg2paths(sys.argv[1])

for p, a in zip(paths, attributes):
    print(p)
    print(a)
    print("-----------------------")
