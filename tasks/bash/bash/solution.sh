#!/bin/sh

print_help() {
    cat <<EOF
Usage: $0 [-n] BLOCKS [PATH...]
Delete regular files until at least BLOCKS 512-byte blocks are freed.

Options:
  -n              dry-run (do not actually delete files)
  -h, --help      display this help and exit

If no PATH is given, the current directory is used.
EOF
}

DRYRUN=0

# разбор опций
while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help)
            print_help
            exit 0
            ;;
        -n)
            DRYRUN=1
            shift
            ;;
        --)
            shift
            break
            ;;
        -*)
            echo "Unknown option: $1" >&2
            print_help
            exit 1
            ;;
        *)
            break
            ;;
    esac
done

if [ $# -lt 1 ]; then
    print_help
    exit 1
fi

BLOCKS="$1"
shift

if [ $# -eq 0 ]; then
    set -- "."
fi

# найти файлы и размеры в блоках
FILES=$(find "$@" -type f ! -xtype l -printf "%b %p\n" 2>/dev/null | sort -nr)

TOTAL=0
echo "$FILES" | while read -r BLK FILE; do
    [ -z "$BLK" ] && continue
    if [ $TOTAL -ge $BLOCKS ]; then
        break
    fi
    echo "$FILE ($BLK blocks)"
    if [ $DRYRUN -eq 0 ]; then
        rm -f -- "$FILE"
    fi
    TOTAL=$((TOTAL + BLK))
done
