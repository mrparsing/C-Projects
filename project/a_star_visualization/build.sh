#!/bin/bash

SRC="a_star.c"
OUT="a_star"

CFLAGS="-Wall -O2"
LDFLAGS="`sdl2-config --cflags --libs` -lm"

echo "Compiling $SRC..."
gcc $CFLAGS $SRC -o $OUT $LDFLAGS

if [ $? -eq 0 ]; then
    echo "Build successful. Run with: ./$OUT"
else
    echo "Build failed."
fi