#!/bin/bash

# Set error flag
set -e

# Get SDL2 include/lib paths from brew
INCLUDE=$(brew --prefix)/include
LIB=$(brew --prefix)/lib

# Compilation flags
FLAGS="-I$INCLUDE -L$LIB -lSDL2 -lSDL2_ttf"

# Build
clang flappy_bird.c $FLAGS -o flappy_bird

# Run
./flappy_bird