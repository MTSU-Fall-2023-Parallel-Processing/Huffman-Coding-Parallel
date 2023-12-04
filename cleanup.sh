echo "##################################################################"
echo "######################### Clean up ###############################"
echo "##################################################################"

# Delete executable
rm -f huffman
echo "Removed huffman executable"

# Delete 'Data' and its files
rm -rf Data
echo "Removed Data and its files"

# Delete 'Encoded' and its files
rm -rf Encoded
echo "Removed Encoded and its files"

# Delete 'Decoded' and its files
rm -rf Decoded
echo "Removed Decoded and its files"
