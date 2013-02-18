#! /usr/bin/python
from __future__ import print_function
import os
import sys

def main():
    dxsdk_dir = os.getenv("DXSDK_DIR")
    if not dxsdk_dir:
        print("DXSDK_DIR is not set. You need to install the DirectX SDK", file=sys.stderr)
        sys.exit(1)
    if dxsdk_dir[1] == ":":
        dxsdk_dir = dxsdk_dir[2:]
    print(dxsdk_dir.replace("\\", "/").strip(), end="")

if __name__ == "__main__":
    main()
