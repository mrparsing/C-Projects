#!/bin/bash

SRC="happy_numbers.c"
OUT="happy_numbers"

CFLAGS="-Wall -O2"

echo "Compiling $SRC..."
gcc $CFLAGS $SRC -o $OUT

if [ $? -eq 0 ]; then
    echo "Build successful. Run with: ./$OUT"
else
    echo "Build failed."
fi