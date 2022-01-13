#!/bin/bash
# -fsanitize=address (not allowed with thread)
SANS="-fsanitize=thread -fsanitize=undefined"
FLAGS="-Wall -Wextra -pedantic"
clang $SANS $FLAGS tests/main.c apg_mod.c -I ./
