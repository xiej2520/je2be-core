#!/usr/bin/env bash
cmake -S . -B Build
cmake --build Build -j 16
# vscode:
#
# "clangd.arguments": [
# 	"--clang-tidy=false",
# 	"--compile-commands-dir=Build",
# ]
