#!/bin/sh
# EXPERIMENTAL lz0 evaluation: packed size and host-side decode speed per method
# usage: bench-lz0.sh [file...]  (defaults to a few /usr/bin executables)
U=build/release/upx
W=${TMPDIR:-/tmp}/lz0eval
mkdir -p "$W" || exit
[ $# -gt 0 ] || set -- /usr/bin/ls /usr/bin/bash /usr/bin/git /usr/bin/vim.basic /usr/bin/gdb
printf 'file\tmethod\torig\tpacked\tratio\tpack_s\tdecode_MB/s\n'
for path in "$@"; do
	f=${path##*/}
	for m in nrv2b nrv2d nrv2e lzma lz0; do
		p=$W/$f.$m
		rm -f "$p"
		s=$(date +%s.%N)
		$U -q -q --best --$m -o "$p" "$path" || { echo "pack failed: $f $m"; continue; }
		e=$(date +%s.%N)
		UPX_BENCH_DECOMPRESS=20 $U -q -q -t "$p" 2>&1 |
		awk -v f=$f -v m=$m -v o=$(stat -c%s "$path") -v p=$(stat -c%s "$p") -v s=$s -v e=$e '
			/UPX_BENCH/ { split($4, a, "="); split($5, b, "="); u += a[2]; t += b[2] }
			/rror/ { print "TEST FAILED: " $0 }
			END { printf "%s\t%s\t%d\t%d\t%.2f%%\t%.1f\t%.0f\n", f, m, o, p, 100 * p / o, e - s, u / (t / 1e3) }'
	done
done
