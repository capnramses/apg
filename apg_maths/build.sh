#!/bin/bash
# -fsanitize=address (not allowed with thread)
SANS="-fsanitize=thread -fsanitize=undefined"
FLAGS="-Wall -Wextra -pedantic"
clang $SANS $FLAGS tests/test.c apg_maths.c -I ./
