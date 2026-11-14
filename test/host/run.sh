#!/bin/sh
# Builds and runs the host unit tests with g++ or clang++.
# Usage: sh test/host/run.sh [--dump FILE]
set -e

dir=$(cd "$(dirname "$0")" && pwd)
lib="$dir/../../src/lib"
out="$dir/build"
mkdir -p "$out"

cxx=${CXX:-$(command -v g++ || command -v clang++)}
"$cxx" -std=c++17 -Wall -Wno-format -I"$lib" -o "$out/host_test" \
  "$dir/test_main.cpp" "$lib/yaml_array.cpp" "$lib/yaml_node.cpp" "$lib/strutil.cpp"

exec "$out/host_test" "$@"
