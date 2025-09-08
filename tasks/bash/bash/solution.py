import argparse
import os
import sys

def blocks512(st):
    # st_blocks обычно уже в 512-байтных блоках
    b = getattr(st, "st_blocks", None)
    if b is None or b == 0:
        return (st.st_size + 511) // 512
    return int(b)

def collect_files(paths):
    if not paths:
        paths = ["."]
    for p in paths:
        if os.path.islink(p):
            continue
        if os.path.isfile(p):
            try:
                st = os.stat(p, follow_symlinks=False)
                yield blocks512(st), p
            except OSError:
                continue
        elif os.path.isdir(p):
            for root, _, files in os.walk(p, followlinks=False):
                for name in files:
                    f = os.path.join(root, name)
                    if os.path.islink(f):
                        continue
                    try:
                        st = os.stat(f, follow_symlinks=False)
                        yield blocks512(st), f
                    except OSError:
                        continue

def main():
    parser = argparse.ArgumentParser(
        prog="solution.py",
        description="Delete regular files until at least BLOCKS 512-byte blocks are freed.",
    )
    parser.add_argument("blocks", type=int, help="number of 512-byte blocks to free")
    parser.add_argument("paths", nargs="*", help="files or directories (default: .)")
    parser.add_argument("-n", action="store_true", help="dry-run (do not actually delete files)")
    args = parser.parse_args()

    files = list(collect_files(args.paths))
    files.sort(reverse=True)  # сортировка по блокам убыванию

    total = 0
    for blk, f in files:
        if total >= args.blocks:
            break
        print(f"{f} ({blk} blocks)")
        if not args.n:
            try:
                os.remove(f)
            except OSError:
                continue
        total += blk

if __name__ == "__main__":
    main()
