#!/bin/bash

# Define build folder
ROOT_DIR="$(pwd)/.."
BUILD_DIR="$(pwd)/../build"

# Build and run project tests
echo
echo "Building and running tests..."
echo
cmake -G "Ninja" -S "$ROOT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"
ctest --test-dir "$BUILD_DIR"

read -p "Press Enter to continue..."
