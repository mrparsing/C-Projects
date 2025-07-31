#!/bin/bash

SRC="curl.c"
OUT="curl"

# Check if libcurl is available
if ! pkg-config --exists libcurl; then
    echo "Error: libcurl not found."
    echo "Install it with:"
    echo "  - Debian/Ubuntu: sudo apt install libcurl4-openssl-dev"
    echo "  - macOS (Homebrew): brew install curl"
    exit 1
fi

CFLAGS="-Wall -O2"
LDFLAGS="$(pkg-config --cflags --libs libcurl)"

echo "Compiling $SRC..."
gcc $CFLAGS $SRC -o $OUT $LDFLAGS

if [ $? -eq 0 ]; then
    echo "Build successful. Run with: ./$OUT"
else
    echo "Build failed."
fi