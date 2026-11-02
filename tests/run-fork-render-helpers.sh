#!/bin/sh
# Run the isolated buffer/edge-index regressions with memory safety checks.
set -eu
root=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=$(mktemp -d)
trap 'rm -rf "${build_dir}"' EXIT HUP INT TERM
"${CXX:-c++}" -std=c++17 -g -O1 -Wall -Wextra -Werror \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    "${root}/tests/fork_render_helpers_test.cpp" -o "${build_dir}/fork-render-helpers"
"${build_dir}/fork-render-helpers"
