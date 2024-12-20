#!/bin/zsh

# Check if a day number argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <day_number>"
    exit 1
fi

# Read the provided day number
DAY_NUMBER=$1

# Define the source folder (day_0) and the new folder (day_n)
TEMPLATE_FOLDER="day_0"
NEW_FOLDER="day_$DAY_NUMBER"

# Check if the source folder (day_0) exists
if [ ! -d "$TEMPLATE_FOLDER" ]; then
    echo "Error: Template folder '$TEMPLATE_FOLDER' does not exist."
    exit 1
fi

# Create the new folder (day_n)
mkdir -p "$NEW_FOLDER"

# Copy main.cpp, CMakeLists.txt etc. from the template folder to the new folder
cp -nv "$TEMPLATE_FOLDER/day_0.cpp" "$NEW_FOLDER/day_$DAY_NUMBER.cpp"
cp -nv "$TEMPLATE_FOLDER/aoc.hpp" "$NEW_FOLDER/"
cp -nv "$TEMPLATE_FOLDER/CMakeLists.txt" "$NEW_FOLDER/"
cp -nv "$TEMPLATE_FOLDER/init_tool_chain.zsh" "$NEW_FOLDER/"
cp -nv "$TEMPLATE_FOLDER/pull_data.zsh" "$NEW_FOLDER/"
cp -nv "$TEMPLATE_FOLDER/pull_text.zsh" "$NEW_FOLDER/"
touch "$NEW_FOLDER/example.txt"
touch "$NEW_FOLDER/example.log"


# Create a day.txt file in the new sub-folder with the provided day number
echo "$DAY_NUMBER" > "$NEW_FOLDER/day.txt"
echo "day.txt created in $NEW_FOLDER with value: $DAY_NUMBER"

# Confirm the files have been copied
echo "You can now cd to $NEW_FOLDER and do ./init_tool_chain.zsh and ./pull_data.zsh"

# Add Git initialization and commit logic
cd "$NEW_FOLDER" || exit 1

# Stage the specified files for Git if they are not already tracked
if git rev-parse --is-inside-work-tree > /dev/null 2>&1; then
    for file in "CMakeLists.txt" "day_$DAY_NUMBER.cpp" "aoc.hpp" "example.txt" "example.log"; do
        if ! git ls-files --error-unmatch "$file" > /dev/null 2>&1; then
            git add "$file"
            echo "Added $file to Git index."
        else
            echo "$file is already tracked by Git."
        fi
    done
    git commit -m "Initial commit for Day $DAY_NUMBER"
    echo "Initial commit made for Day $DAY_NUMBER"
else
    echo "Not inside a Git repository. Skipping Git commit."
fi
