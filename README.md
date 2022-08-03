                            Assignment 7 (CSE 13S by Prof. Max Dunne)
                                            
                                    Lempel-Ziv Compression
                                       

In the program, I have constructed two C program solutions working for compressing and decompressing data. I utilize Lemple-Ziv Compression this time, in order to represent repeated patterns in data with using pairs which are each comprised of a code and a symbol while a code is an unsigned 16-bit integer and a symbol is a 8-bit ASCII character. 

Since I have already implemeted a Makefile for users, so users can simply use "make" command in the work dictionary and it will help to compile all .c files together intentionally."make clean" will remove and delete all compiled file, such as .o.  "make infer" will help to analyze the program. "make valgrind" can help to analyze the memory leak situtaion.

Usage for both encode and decode:
    After run the make command, all compiled files are ready to go.

    1. -v: Display compression statistics
        *Compressed file size: X bytes
        *Uncompressed file size: X bytes 
        *Compression ratio: XX.XX%

        
    2. -i <input>: Specify input to compress(decompressed)(stdin by default)

    3. -o <output>: Specify output of compressed(decompressed) input(stdout by default)

    For example, user can use command "./decode < fox.encode" standing for decoding the file fox.encode.

make infer error: 1 memory leak from decode.c, which is a given code from manual

Author: Zhuo Xiao






