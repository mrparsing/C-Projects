#!/bin/bash

SRC="rsa.c"
OUT="rsa"

CFLAGS="-Wall -O2"
LIBS="-lgmp"

echo "Compiling $SRC..."
gcc $CFLAGS $SRC -o $OUT $LIBS

if [ $? -eq 0 ]; then
    echo "Build successful. Run with: ./$OUT"
else
    echo "Build failed."
fi