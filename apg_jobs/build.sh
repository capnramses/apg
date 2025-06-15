#!/bin/sh

# any error code causes script to exit with error code
set -e

# -fsanitize=address (not allowed with thread)
SANS="-fsanitize=thread -fsanitize=undefined"
FLAGS="-Wall -Wextra -pedantic"
clang $SANS $FLAGS tests/main.c apg_jobs.c -I ./ -pthread
