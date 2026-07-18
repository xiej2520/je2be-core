#!/usr/bin/env bash
cmake -S . -B Build -G Ninja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake --build Build -j 16
# vscode:
#
# "clangd.arguments": [
# 	"--clang-tidy=false",
# 	"--compile-commands-dir=Build",
# ]
