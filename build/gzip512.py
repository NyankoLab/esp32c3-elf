import sys
import zlib

if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} <input> <output.gz>")
    sys.exit(1)

in_path = sys.argv[1]
out_path = sys.argv[2]

with open(in_path, "rb") as f:
    data = f.read()

compressed = zlib.compress(data, level=9, wbits=16 + 9)

with open(out_path, "wb") as f:
    f.write(compressed)
