#!/bin/bash

# Generate file with random characters
generate_random_file ()
{
    local file_name=$1
    local size=$2
    
    base64 /dev/urandom | head -c $size > Data/$file_name

    echo "Created Data/$file_name"
}

echo "Creating 'Data' folder and files with random content inside..."

# Create 'Data' folder
mkdir Data

# Create files with given names and byte sizes inside 'Data'
generate_random_file "file_100KB.txt" 102400
generate_random_file "file_100MB.txt" 104857600
generate_random_file "file_500KB.txt" 512000
generate_random_file "file_500MB.txt" 524288000

echo "Setup completed. All files are in the 'Data' folder."
