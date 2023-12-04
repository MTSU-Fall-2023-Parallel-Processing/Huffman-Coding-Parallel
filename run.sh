#!/bin/bash

echo "##################################################################"
echo "######################### Compile code ###########################"
echo "##################################################################"

# Parallel file
parallel_file="huffman.c"

# Parallel executable
parallel_executable="huffman"

# Check if file exists
if [ ! -e "$parallel_file" ]; then
    echo "Error: $parallel_file file not found."
    exit 1
fi

# Compile the parallel program
gcc -o "$parallel_executable" "$parallel_file" -pthread

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable: $parallel_executable"
else
    echo "Error: Compilation failed for $parallel_executable."
fi

echo "##################################################################"
echo "########################### Encode ###############################"
echo "##################################################################"

# Folder path
folder_path="Data"

# Create 'Encoded' folder
mkdir Encoded

# Check if the folder exists
if [ -d "$folder_path" ]; then
    # Loop through all files in the folder
    # run with 1 thread
    for file in "$folder_path"/*; do
        if [ -f "$file" ]; then
            echo -n "Encoding huffman with file: $(basename "$file") and 1 thread : "
            ./huffman encode "$file" "Encoded/encoded_$(basename "$file" | cut -d. -f1).dat" 1
        fi
    done

    # run with 16 thread
    for file in "$folder_path"/*; do
        if [ -f "$file" ]; then
            echo -n "Encoding huffman with file: $(basename "$file") and 16 thread : "
            ./huffman encode "$file" "Encoded/encoded_$(basename "$file" | cut -d. -f1).dat" 16
        fi
    done

    # run with 32 thread
    for file in "$folder_path"/*; do
        if [ -f "$file" ]; then
            echo -n "Encoding huffman with file: $(basename "$file") and 32 thread : "
            ./huffman encode "$file" "Encoded/encoded_$(basename "$file" | cut -d. -f1).dat" 32
        fi
    done
else
    echo "Error: The folder '$folder_path' does not exist."
fi

echo "##################################################################"
echo "############################ Decode ##############################"
echo "##################################################################"

# Folder path
folder_path="Encoded"

# Create 'Decoded' folder
mkdir Decoded

# Check if the folder exists
if [ -d "$folder_path" ]; then
    # Loop through all files in the folder
    # run with 1 thread
    for file in "$folder_path"/*; do
        if [ -f "$file" ]; then
            echo -n "Decoding huffman with file: $(basename "$file") : "
            ./huffman decode "$file" "Decoded/decoded_$(basename "$file" | cut -d. -f1)).txt" 1
        fi
    done
else
    echo "Error: The folder '$folder_path' does not exist."
fi

echo "##################################################################"
echo "##################### Check Difference ###########################"
echo "##################################################################"

# Folder paths
source_data="Data"
source_decoded="Decoded"

# Numbers used to trim file names for comparison
file2_prefix_length=16
file2_suffix_length=1

# Get a list of files from both folders
source_data=("$source_data"/*)
source_decoded=("$source_decoded"/*)

# Iterate through all files in 'Data'
for file1 in "${source_data[@]}"; do
    # Extract file name without path and extention
    file_name1=$(basename "$file1" | cut -d. -f1)

    # Iterate through files in 'Decoded'
    for file2 in "${source_decoded[@]}"; do
        # Extract the file name without path and extention
        file_name2=$(basename "$file2" | cut -d. -f1)

        # Trim the file name based on the prefix and suffix length
        file_name2="${file_name2:$file2_prefix_length:$(( ${#file_name2} - $file2_prefix_length - $file2_suffix_length ))}"

        # Check if the names match
        if [ "$file_name1" = "$file_name2" ]; then
            # Perform a diff on the matching files
            echo "Performing diff between $file1 and $file2"
            diff "$file1" "$file2"
        fi
    done
done

echo "Comparisons completed"
