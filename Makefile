CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic

default: all

all: encode decode

encode: code.h encode.c io.h io.c trie.h trie.c
	$(CC) $(CFLAGS) encode.c io.c trie.c -o encode -lm

decode: code.h decode.c io.h io.c word.h word.c
	$(CC) $(CFLAGS) decode.c io.c word.c -o decode -lm

clean:
	rm -rf encode encode.o decode decode.o trie.o word.o io.o infer-out

valgrind: 
	valgrind ./encode ./decode

clang:
	clang-format -i -style=file *.c

infer:
	make clean; infer-capture -- make; infer-analyze -- make
