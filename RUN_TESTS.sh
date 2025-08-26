#!/bin/bash
echo "Running all tests..."

# Define build and logs folder
BUILD_DIR="$(pwd)/build"
BIN_DIR="$BUILD_DIR/bin"
LOGS_DIR="$(pwd)/logs"

# Create logs folder if it doesn't exist
mkdir -p "$LOGS_DIR"

# Build project tests
echo
echo "Building tests..."
echo
cmake -G "Ninja" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"

# Loop through all exe files in build/bin folder and output the results in logs folder
echo
for exe in "$BIN_DIR"/*; do
    if [ -f "$exe" ]; then
        name=$(basename "$exe")
        echo "Running $name ..."
        "$exe" > "$LOGS_DIR/$name.txt" 2>&1
    fi
done

echo
echo "All tests finished."
read -p "Press Enter to continue..."
