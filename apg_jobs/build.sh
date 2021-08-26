#!/bin/bash
# -fsanitize=address (not allowed with thread)
SANS="-fsanitize=thread -fsanitize=undefined"
FLAGS="-Wall -Wextra -pedantic"
clang $SANS $FLAGS main.c apg_jobs.c -pthread
