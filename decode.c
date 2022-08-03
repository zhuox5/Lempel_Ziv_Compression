#include "trie.h"
#include "code.h"
#include "io.h"
#include "word.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define OPTIONS "vi:o:"

void decompress(int infile, int outfile){
    WordTable* table = wt_create();
    if(table != NULL){
        uint8_t curr_sym = 0;
        uint16_t curr_code = 0;
        uint16_t next_code = START_CODE;

        while(read_pair(infile, &curr_code, &curr_sym, ((uint8_t) floor(log2(next_code))) + 1)){
            table[next_code] = word_append_sym(table[curr_code], curr_sym);
            if(table[next_code] != NULL && outfile >= 0){
               buffer_word(outfile, table[next_code]);
                //++next_code; 
            }
            next_code += 1;

            if(next_code == MAX_CODE){
                wt_reset(table);
                next_code = START_CODE;
            }
        }
    }
    flush_words(outfile);
    wt_delete(table);
}

int main(int argc, char* argv[]){
    int opt;
    char* infile = NULL;
    char* outfile = NULL;
    bool show_flag = false;

    while((opt = getopt (argc , argv , OPTIONS)) != -1){
        switch(opt){
            case 'v':
                show_flag = true;
                break;
            case 'i':
                infile = optarg;
                break;
            case 'o':
                outfile = optarg;
                break;
            default:
                exit(1);
        }
    }

    if(optind < argc){
        //Check for extra arguments and print them out
        for(; optind < argc; optind++){      
            printf("Extra arguments: %s\n", argv[optind]);  
        } 
        return 0; 
    }

    int input = STDIN_FILENO;
    if(infile != NULL){
        input = open(infile, O_RDONLY);
        if(input < 0){
            exit(1);
        }
    }

    FileHeader fh;
    read_header(input, &fh);
    if(fh.magic != MAGIC){
        close(input);
        exit(1);
    }

    int output = STDOUT_FILENO;
    if(outfile != NULL){
        output = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, fh.protection);
        if(output < 0){
            close(input);
            exit(1);
        }
    }

    decompress(input, output);

    //Derived from posts on Piazza
    if(show_flag){
        struct stat srcstats;
        struct stat dststats;

        fstat(input, &srcstats);
        fstat(output, &dststats);

        printf("Compressed file size: %lld bytes\n", srcstats.st_size);
        printf("Uncompressed file size: %lld bytes\n", dststats.st_size);
        printf("Compression ratio: %.2f%%\n", 100 * (1 - ((double)srcstats.st_size / dststats.st_size)));
    }

    close(input);
    close(output);
    return 0;
}
