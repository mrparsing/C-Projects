#!/bin/bash

# Set error flag
set -e

# Get SDL2 include/lib paths from brew
INCLUDE=$(brew --prefix)/include
LIB=$(brew --prefix)/lib

# Compilation flags
FLAGS="-I$INCLUDE -L$LIB -lSDL2 -lSDL2_ttf"

# Build
clang graph_visualizer.c $FLAGS -o graph_visualizer

# Run
./graph_visualizer