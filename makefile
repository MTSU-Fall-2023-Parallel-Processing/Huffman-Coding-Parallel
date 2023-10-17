huffman: huffman.c
	gcc huffman.c -o huffman

clean:
	rm -f l2 rm -f  *.dat huffman decoded.txt

run:
	./huffman encode untitled.txt encoded.dat

run2:
	./huffman decode encoded.dat decoded.txt

test:
	diff decoded.txt untitled.txt
