#!/bin/zsh

# Get the generator name from the parent folder's tool_chain.txt
GENERATOR_NAME=$(cat ../tool_chain.txt)

# Check if a generator name was read from the tool_chain.txt
if [ -z "$GENERATOR_NAME" ]; then
    echo "Error: No generator name found in '../tool_chain.txt'."
    exit 1
fi

# Define the sub-folder name based on the generator name
BUILD_FOLDER="$GENERATOR_NAME"

# Create the sub-folder with the generator name
mkdir -p "$BUILD_FOLDER"

# Change into the newly created sub-folder
cd "$BUILD_FOLDER" || exit

# Run CMake with the generator name as the argument
cmake -G "$GENERATOR_NAME" ../