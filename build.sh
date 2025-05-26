#!/usr/bin/env bash
#cmake -S . -B Build -G Ninja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake --build Build -j 16
#cmake -S . -B build-asan \
#  -DCMAKE_BUILD_TYPE=Debug \
#  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g" \
#  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
#cmake --build build-asan -j 16
#clear; ASAN_OPTIONS=print_stacktrace=1 ./build-asan/p2j -o scratch/ps3convert/1.83out -i scratch/ps3world/-2878103199665976685EndOuterIsland\ NPEB01899--250317051137/
# vscode:
#
# "clangd.arguments": [
# 	"--clang-tidy=false",
# 	"--compile-commands-dir=Build",
# ]