#!/bin/bash
gcc -o play_file.bin \
examples/main_play_file.c \
apg_wav.c \
-I ./ \
-lportaudio
