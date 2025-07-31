#!/bin/bash

SRC="keylogger.c"
OUT="keylogger"
CFLAGS="-Wall -O2"

HEADER_PATH="/usr/include/linux/input.h"

echo "Compiling $SRC..."

# Controllo se il file linux/input.h esiste
if [ ! -f "$HEADER_PATH" ]; then
    echo "Error: '$HEADER_PATH' not found."
    echo "Please install the appropriate Linux kernel headers."
    echo "For Debian/Ubuntu: sudo apt install linux-libc-dev"
    echo "For Arch: sudo pacman -S linux-headers"
    echo "For Fedora: sudo dnf install kernel-headers"
    exit 1
fi

gcc $CFLAGS $SRC -o $OUT

if [ $? -eq 0 ]; then
    echo "Build successful. Run with: ./$OUT"
else
    echo "Build failed."
fi