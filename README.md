# Huffman-Coding-Parallel

Data compression is an important process to allow for memory savings. There are two main types of compression algorithms lossless and lossy. Lossless algorithms ensure that there is no loss of data during the encoding and decoding of data. Lossy can result in lost data after encoding and decoding data. We will be exploring the parallelization of Huffman coding a popular lossless compression algorithm. The goal of this project is to improve the speed of encoding or decoding data using Huffman coding. 

<hr>

In this project, we implement the Huffman Coding algorithm in Serial and Parallel. 

## Source code

* `huffman.c` - A C programming language implementation.
* `untitled.txt`- A large text file consisting of 'Lorem Ipsum' text for testing.
* `makefile` - For compiling and running tests.

## Usage

Compile the executible

    $ make

Encode `untitled.txt` into `encoded.dat`

    $ make run

Decode `encoded.dat` into `decoded.txt`

    $ make run2

Compare `untitled.txt` to `decoded.txt`

    $ make test
