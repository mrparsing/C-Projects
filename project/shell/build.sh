#!/bin/bash

SRC="shell.c"
OUT="shell"

CFLAGS="-Wall -O2"
LIBS="-lncurses"

echo "Compiling $SRC..."
gcc $CFLAGS $SRC -o $OUT $LIBS

if [ $? -eq 0 ]; then
    echo "Build successful. Run with: ./$OUT"
else
    echo "Build failed."
fi