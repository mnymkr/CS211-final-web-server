#!/bin/bash

# DIRECTORY="/mnt/c/Users/nguye/Documents/Project/CSAudioService/src/"  # Change this to your desired directory
DIRECTORY="../test_IO"
# Check if the provided argument is a valid directory
if [ ! -d "$DIRECTORY" ]; then
    echo "Error: $DIRECTORY is not a valid directory."
    exit 1
fi

# Find all files in the specified directory and run ./client for each file
find "$DIRECTORY" -type f | while read -r filename; do
    echo "Running client for file: $filename"
    time ./client_IO "$filename">/dev/null &
done