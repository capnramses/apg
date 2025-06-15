#!/bin/bash

# any error code causes script to exit with error code (to its parent i.e the build all script).
set -e

CC=clang
CPP=clang++
LD=ld
FLAGS="-g -Wall -Wextra -pedantic -Werror"
# -fsanitize=address (not allowed with thread)
SANS="-fsanitize=thread -fsanitize=undefined"
$CC -o print_tracks.bin $SANS $FLAGS tests/print_tracks.c apg_mod.c -I ./

APG_WAV_INC="../apg_wav/"

$CC -o dump_wavs.bin $SANS $FLAGS tests/dump_wavs.c apg_mod.c -I ./ \
-I $APG_WAV_INC $APG_WAV_INC/apg_wav.c
