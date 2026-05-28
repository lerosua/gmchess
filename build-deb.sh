#!/usr/bin/env bash

set -euo pipefail

app_name="gmchess"
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
parent_dir="$(dirname "$script_dir")"
build_dir="$parent_dir/builds"

cd "$script_dir"

cleanup() {
	debian/rules clean >/dev/null 2>&1 || true
}
trap cleanup EXIT

mkdir -p "$build_dir"
find "$build_dir" -maxdepth 1 -type f \( \
	-name "${app_name}_*.deb" -o \
	-name "${app_name}-dbgsym_*.ddeb" -o \
	-name "${app_name}_*.buildinfo" -o \
	-name "${app_name}_*.changes" \
\) -delete

dpkg-buildpackage -us -uc -b

find "$parent_dir" -maxdepth 1 -type f \( \
	-name "${app_name}_*.deb" -o \
	-name "${app_name}-dbgsym_*.ddeb" -o \
	-name "${app_name}_*.buildinfo" -o \
	-name "${app_name}_*.changes" \
\) -exec mv -t "$build_dir" {} +

ls -lh "$build_dir"
