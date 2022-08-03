#include "io.h"
#include "code.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ALPHABET 256
#define BLOCK    4096 

uint8_t sym_buffer[BLOCK] = {0};            // symbol buffer
int sym_index = 0;                          // current symbol to be processed
int total_syms = 0;                         // symbols in buffer, and i have remove the extern one in io.h

uint8_t bit_buffer[BLOCK] = {0};            // bit buffer
int bit_index = 0;                          // current bit to be processed
int total_bits;                             // bits in buffer

static void pack_bit(int index, bool bits) {
    if(bits){                                                    // 1
        bit_buffer[index / 8] |= (1u << (index % 8));
    }else{                                                       // 0
        bit_buffer[index / 8] &= ~(1u << (index % 8));
    }
}

// Unpack a bit from bit buffer
static bool unpack_bit(int index){
    bool bits = bit_buffer[index / 8] & (1u << (index % 8));
    return bits;
}
  
// Struct definition of a FileHeader.
//
// magic:       Magic number indicating a file compressed by this program.
// protection:  Protection/permissions of the original, uncompressed file.
//

//typedef struct FileHeader {
//  uint32_t magic;
//  uint16_t protection;
//} FileHeader;// Wrapper for the read() syscall.

// Loops to read the specified number of bytes, or until input is exhausted.
// Returns the number of bytes read.
//
// infile:  File descriptor of the input file to read from.
// buf:     Buffer to store read bytes into.
// to_read: Number of bytes to read.
// returns: Number of bytes read.
//
int read_bytes(int infile, uint8_t *buf, int to_read){
    int read_len = 0;

    while (read_len < to_read) {
        int len = read(infile, &buf[read_len], to_read - read_len);
        if (len <= 0) {
            break;
        }
        read_len += len;
    }

    return read_len;
}

//
// Wrapper for the write() syscall.
// Loops to write the specified number of bytes, or until nothing is written.
// Returns the number of bytes written.
//
// outfile:   File descriptor of the output file to write to.
// buf:       Buffer that stores the bytes to write out.
// to_write:  Number of bytes to write.
// returns:   Number of bytes written.
//
int write_bytes(int outfile, uint8_t *buf, int to_write){
    int wbytes = 0;
    while(wbytes < to_write){
        int total = write(outfile, &buf[wbytes], to_write - wbytes);
        if(total < 0){                   //error or in the EOF
            break;
        }
        wbytes += total;
    }
    return wbytes;
}

//
// Reads in a FileHeader from the input file.
//
// infile:  File descriptor of input file to read header from.
// header:  Pointer to memory where the bytes of the read header should go.
// returns: Void.
//

//Hint: use open in main() to gain descriptor (int infile)
void read_header(int infile, FileHeader *header){
    int len = read_bytes(infile, (uint8_t*)header, sizeof(FileHeader));
    if (len != sizeof(FileHeader)) {
        return;
    }
    /*struct stat stats;
    fstat(infile, &stats);
    uint8_t buffer[BLOCK] = { 0 };
    int header_length = read_bytes(infile, buffer, sizeof(FileHeader));
    read(infile, buffer, header_length);

    int srcfile = header->protection;
    int dstfile = read_bytes(infile, buffer, sizeof(FileHeader));

    struct stat srcstats;
    struct stat dststats;

    fstat(srcfile, &srcstats);
    fstat(dstfile, &dststats);
    fchmod(dstfile, srcstats.st_mode);*/

}

//
// Writes a FileHeader to the output file.
//
// outfile: File descriptor of output file to write header to.
// header:  Pointer to the header to write out.
// returns: Void.
//
void write_header(int outfile, FileHeader *header){
    int len = write_bytes(outfile, (uint8_t*)header, sizeof(FileHeader));
    if (len != sizeof(FileHeader)) {
        return;
    }
}

//
// "Reads" a symbol from the input file.
// The "read" symbol is placed into the pointer to sym (pass by reference).
// In reality, a block of symbols is read into a buffer.
// An index keeps track of the currently read symbol in the buffer.
// Once all symbols are processed, another block is read.
// If less than a block is read, the end of the buffer is updated.
// Returns true if there are symbols to be read, false otherwise.
//
// infile:  File descriptor of input file to read symbols from.
// sym:     Pointer to memory which stores the read symbol.
// returns: True if there are symbols to be read, false otherwise.
//
bool read_sym(int infile, uint8_t *sym){

    if(sym_index < total_syms){         //if not full
        *sym = sym_buffer[sym_index++];
        return true;
    }
    memset(sym_buffer, 0, BLOCK);
    total_syms = read_bytes(infile, sym_buffer, BLOCK);
    if(total_syms > 0){
        sym_index = 0;
        *sym = sym_buffer[sym_index++];
        return true;
    }
    return false;
}

//
// Buffers a pair. A pair is comprised of a symbol and an index.
// The bits of the symbol are buffered first, starting from the LSB.
// The bits of the index are buffered next, also starting from the LSB.
// bit_len bits of the index are buffered to provide a minimal representation.
// The buffer is written out whenever it is filled.
//
// outfile: File descriptor of the output file to write to.
// sym:     Symbol of the pair to buffer.
// index:   Index of the pair to buffer.
// bit_len: Number of bits of the index to buffer.
// returns: Void.
//
void buffer_pair(int outfile, uint16_t code, uint8_t sym, uint8_t bit_len){
    uint32_t data = (sym << bit_len) | code;
    uint32_t num_bits = 8 + bit_len;
    uint32_t remain_bits = BLOCK * 8 - bit_index;

    uint32_t index = 0;

    while((index < num_bits) && (index < remain_bits)){
        pack_bit(bit_index++, data & (1u << index));
        ++index;
    }

    if(remain_bits > index){
        return;
    }

    int wbytes = write_bytes(outfile, bit_buffer, BLOCK);       // buffer is full, write it
    if(wbytes != BLOCK){
        return;
    }
    //memset(bit_buffer, 0, BLOCK);
    bit_index = 0;                              // reset

    while (index < num_bits)
    {
        pack_bit(bit_index++, data & (1u << index));
        ++index;
    }
    
}

//
// Writes out any remaining pairs of symbols and indexes to the output file.
//
// outfile: File descriptor of the output file to write to.
// returns: Void.
//
void flush_pairs(int outfile){
    if(bit_index == 0){
        return;
    }
    int need_bytes = (bit_index - 1) / 8 + 1;
    int wbytes = write(outfile, bit_buffer, need_bytes);
    if(wbytes != need_bytes){
        return;
    }
    memset(bit_buffer, 0, BLOCK);
    bit_index = 0;              //reset?
}

//
// "Reads" a pair (symbol and index) from the input file.
// The "read" symbol is placed in the pointer to sym (pass by reference).
// The "read" index is placed in the pointer to index (pass by reference).
// In reality, a block of pairs is read into a buffer.
// An index keeps track of the current bit in the buffer.
// Once all bits have been processed, another block is read.
// The first 8 bits of the pair constitute the symbol, starting from the LSB.
// The next bit_len bits constitutes the index, starting from the the LSB.
// Returns true if there are pairs left to read in the buffer, else false.
// There are pairs left to read if the read index is not STOP_INDEX.
//
// infile:  File descriptor of the input file to read from.
// sym:     Pointer to memory which stores the read symbol.
// index:   Pointer to memory which stores the read index.
// bit_len: Length in bits of the index to read.
// returns: True if there are pairs left to read, false otherwise.
//
bool read_pair(int infile, uint16_t *code, uint8_t *sym, uint8_t bit_len){

    uint32_t num_bits = 8 + bit_len;
    uint32_t remian_bits = total_bits - bit_index;
    uint32_t data = 0;

    uint32_t index = 0;
    while(index < num_bits && index < remian_bits){
        data |= (unpack_bit(bit_index++) << index);
        ++index;
    }

    if (index >= remian_bits) {
        memset(bit_buffer, 0, BLOCK);
        total_bits = read_bytes(infile, bit_buffer, BLOCK);
   
        if (total_bits <= 0) {
            return false;
        }
        total_bits *= 8;
        bit_index = 0;

        while(index < num_bits){
            data |= (unpack_bit(bit_index++) << index);
            ++index;
        }
    }

    *code = data & ~(0xffffffff << bit_len);                //reversed
    if (*code == STOP_CODE) {
        return false;
    }
    *sym = (data >> bit_len) & 0xff;

    return true;
}

//
// Buffers a Word, or more specifically, the symbols of a Word.
// Each symbol of the Word is placed into a buffer.
// The buffer is written out when it is filled.
//
// outfile: File descriptor of the output file to write to.
// w:       Word to buffer.
// returns: Void.
//
void buffer_word(int outfile, Word *w){

    uint32_t remain_bytes = BLOCK - sym_index;
    uint32_t index = 0;
    while (index < remain_bytes && index < w->len)
    {
        sym_buffer[sym_index++] = w->syms[index];
        ++index;
    }
    if (index < remain_bytes) {
        return;
    }
    int wbytes = write_bytes(outfile, sym_buffer, BLOCK);
    if (wbytes != BLOCK) {
        return;
    }

    memset(sym_buffer, 0, BLOCK);
    sym_index = 0;                          //reset
    while(index < w->len){
        sym_buffer[sym_index++] = w->syms[index];
        ++index;
    }
}

//
// Writes out any remaining symbols in the buffer.
//
// outfile: File descriptor of the output file to write to.
// returns: Void.
//
void flush_words(int outfile){

    if (sym_index != 0) {
        int wbytes = write_bytes(outfile, sym_buffer, sym_index);
        if (wbytes != sym_index) {
            return;
        }
        sym_index = 0;
    }
    else{
        return;
    }

}
