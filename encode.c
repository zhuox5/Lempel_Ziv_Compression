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

//Derived from lab manual of asgn7
void encode_compress(int infile, int outfile){

    FileHeader fh;
    fh.magic = MAGIC;           //protection
    struct stat stats;
    fstat(infile, &stats);      //Derived from Piazza.com of CSE13S and credited by staffs
    fh.protection = stats.st_mode;

    write_header(outfile, &fh);

    TrieNode* root = trie_create();
    TrieNode* curr_node = root;
    if(curr_node == NULL){
        return;                 //no valid
    }
    TrieNode* prev_node = NULL;
    uint8_t curr_sym = 0;
    uint8_t prev_sym = 0;
    uint16_t next_code = START_CODE;

    while (read_sym(infile, &curr_sym)) {
        TrieNode* next_node = trie_step(curr_node, curr_sym);

        if(next_node == NULL){
            buffer_pair(outfile, curr_node->code, curr_sym, ((uint8_t)floor(log2(next_code))) + 1);
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            ++next_code;
        }else{
            prev_node = curr_node;
            curr_node = next_node;
        }

        if(next_code == MAX_CODE){
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }
        prev_sym = curr_sym;
    }
    if(curr_node != root && prev_node){
        buffer_pair(outfile, prev_node->code, prev_sym, ((uint8_t)floor(log2(next_code))) + 1);
        next_code = (next_code + 1) % MAX_CODE;
    }

    buffer_pair(outfile, STOP_CODE, 0, ((uint8_t)floor(log2(next_code))) + 1);
    flush_pairs(outfile);
    trie_delete(root);
}

int main(int argc, char* argv[]){

    int opt;
    char* infile = NULL;
    char* outfile = NULL;
    bool show_flag = false;

    while ((opt = getopt (argc , argv , OPTIONS)) != -1) {
        switch (opt) {
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

    if (optind < argc) {
        //Check for extra arguments and print them out
        for(; optind < argc; optind++){      
            printf("Extra arguments: %s\n", argv[optind]);  
        } 
        return 0; 
    }

    int input = STDIN_FILENO;
    if (infile != NULL) {
        input = open(infile, O_RDONLY);
        if (input < 0) {
            exit(1);
        }
    }

    int output = STDOUT_FILENO;
    if (outfile != NULL) {
        output = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU);
        if (output < 0) {
            close(input);
            exit(1);
        }
    }

    encode_compress(input, output);
    
    //Derived from posts on Piazza
    if (show_flag) {
        struct stat srcstats;
        struct stat dststats;

        fstat(input, &srcstats);
        fstat(output, &dststats);

        printf("Compressed file size: %lld bytes\n", dststats.st_size);
        printf("Uncompressed file size: %lld bytes\n", srcstats.st_size);
        printf("Compression ratio: %.2f%%\n", 100 * (1 - ((double)dststats.st_size / srcstats.st_size)));
    }

    close(input);
    close(output);
    return 0;
}
