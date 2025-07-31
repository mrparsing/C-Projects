#!/bin/bash

SRC="collatz_conjecture.c"
OUT="collatz_conjecture"

CFLAGS="-Wall -O2"

echo "Compiling $SRC..."
gcc $CFLAGS $SRC -o $OUT $LDFLAGS

if [ $? -eq 0 ]; then
    echo "Build successful. Run with: ./$OUT"
else
    echo "Build failed."
fi