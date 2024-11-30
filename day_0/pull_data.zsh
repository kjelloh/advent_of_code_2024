#!/bin/zsh

# Read the session cookie from the file
SESSION_COOKIE=$(cat ../cookie.txt)

# Read the year from the year.txt file
YEAR=$(cat ../year.txt)

# Read the day number from the day.txt file
DAY_NUMBER=$(cat day.txt)

curl -H "Cookie: session=$SESSION_COOKIE" -o "puzzle.txt" https://adventofcode.com/$YEAR/day/$DAY_NUMBER/input
cat "puzzle.txt"

