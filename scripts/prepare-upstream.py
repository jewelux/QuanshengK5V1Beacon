#!/usr/bin/env python3
"""Assemble a pinned V1/V3 source tree; never overwrite an existing destination."""
import argparse
from pathlib import Path
import shutil
import subprocess

BASES = {
    "V1": ("https://github.com/reald/uv-k5-firmware-custom.git", "5955ccfc8732f4a16b628276ed5fa98f2db54e55"),
    "V3": ("https://github.com/reald/uv-k1-k5v3-firmware-custom.git", "97b1890bed9628f625d787bfa42683f2da912614"),
}


def overlay(source, destination):
    shutil.copytree(source, destination, dirs_exist_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--hardware", choices=BASES, default="V1")
    parser.add_argument("--destination", required=True, type=Path)
    args = parser.parse_args()
    destination = args.destination.resolve()
    if destination.exists():
        parser.error(f"Destination already exists: {destination}")
    repository, revision = BASES[args.hardware]
    subprocess.run(["git", "clone", repository, str(destination)], check=True)
    subprocess.run(["git", "-C", str(destination), "checkout", "--detach", revision], check=True)
    root = Path(__file__).resolve().parent.parent
    if args.hardware == "V1":
        overlay(root / "source", destination)
    else:
        overlay(root / "source-v3", destination)
        for name in ("beacon.c", "beacon.h"):
            shutil.copy2(root / "source/app" / name, destination / "App/app" / name)
    print(f"Prepared {args.hardware} source tree: {destination}")
    print("Build with: " + ("make ENABLE_BEACON_MO=1 ENABLE_PREVENT_TX=0 ENABLE_ARDF=0 ENABLE_SPECTRUM=0 ENABLE_FMRADIO=0"
                            if args.hardware == "V1" else "cmake --preset Beacon && cmake --build --preset Beacon"))


if __name__ == "__main__":
    main()
