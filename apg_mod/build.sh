#!/bin/bash
# -fsanitize=address (not allowed with thread)
SANS="-fsanitize=thread -fsanitize=undefined"
FLAGS="-Wall -Wextra -pedantic"
clang -o print_tracks.bin $SANS $FLAGS tests/print_tracks.c apg_mod.c -I ./

clang -o play_song.bin $SANS $FLAGS examples/play_song.c apg_mod.c -I ./ -lportaudio

APG_WAV_INC="../apg_wav/"

clang -o dump_wavs.bin $SANS $FLAGS tests/dump_wavs.c apg_mod.c -I ./ \
-I $APG_WAV_INC $APG_WAV_INC/apg_wav.c
