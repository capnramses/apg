#!/bin/bash
BIN=fuzz_32bpp
INDIR=fuzzing_inputs/32bpp
OUTDIR=fuzz_32bpp_outputs

#set up folders
rm -f $BIN
rm -rf $OUTDIR
mkdir $OUTDIR

#build for fuzzing
afl-gcc -g -o $BIN fuzz_main.c ../apg_bmp.c -I ../

#run with fuzz
AFL_EXIT_WHEN_DONE=1 AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
AFL_SKIP_CPUFREQ=1 afl-fuzz -i $INDIR/ -o $OUTDIR/ -- ./$BIN @@
