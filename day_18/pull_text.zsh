#!/bin/zsh

# Read the session cookie from the file
SESSION_COOKIE=$(cat ../cookie.txt)

# Read the year from the year.txt file
YEAR=$(cat ../year.txt)

# Read the day number from the day.txt file
DAY_NUMBER=$(cat day.txt)

# Define the output file name
OUTPUT_FILE="day_${DAY_NUMBER}.txt"

# Fetch the puzzle description webpage using curl
curl -s -H "Cookie: session=$SESSION_COOKIE" \
    -o "puzzle.html" "https://adventofcode.com/$YEAR/day/$DAY_NUMBER"

# Use lynx to convert the HTML to plain text and save to the output file
lynx -dump -nolist puzzle.html > "$OUTPUT_FILE"

# Optional: Remove the raw HTML file if you don't need it
rm puzzle.html

# Display a success message
echo "Puzzle description saved to $OUTPUT_FILE"
