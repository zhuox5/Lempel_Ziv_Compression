#include "word.h"
#include "code.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <memory.h>

// Constructor for a word.
//
// syms:    Array of symbols a Word represents.
// len:     Length of the array of symbols.
// returns: Pointer to a Word that has been allocated memory.
//
Word *word_create(uint8_t *syms, uint32_t len){
    Word *w = (Word *)malloc(sizeof(Word));
    if(w){
        w->len = len;
        if(!(len > 0)){
            w->syms = NULL;         //set values
            return w;
        }
        w->syms = malloc(sizeof(uint8_t) * len);
        if(w->syms == NULL){
            free(w);
            return NULL;
        }
        memcpy(w->syms, syms, len);
        return w;
    }   
    return NULL;
}

//
// Constructs a new Word from the specified Word appended with a symbol.
// The Word specified to append to may be empty.
// If the above is the case, the new Word should contain only the symbol.
//
// w:       Word to append to.
// sym:     Symbol to append.
// returns: New Word which represents the result of appending.
//
Word *word_append_sym(Word *w, uint8_t sym){

    Word* temp = malloc(sizeof(Word));
    if (temp == NULL) {
        exit(1);
    }
    temp->len = w->len + 1;                     // +1 for append
    temp->syms = malloc(sizeof(uint8_t) * temp->len);
    if (!temp->syms) {
        free(temp);                             //No valid if is NULL
        exit(1);
    }
    memcpy(temp->syms, w->syms, w->len);
    temp->syms[w->len] = sym;
    return temp;  
}

//
// Destructor for a Word.
//
// w:       Word to free memory for.
// returns: Void.
//
void word_delete(Word *w){
    if(w){
        free(w->syms);
        free(w);
    }
}

//
// Creates a new WordTable, which is an array of Words.
// A WordTable has a pre-defined size of MAX_CODE (UINT16_MAX - 1).
// This is because codes are 16-bit integers.
// A WordTable is initialized with a single Word at index EMPTY_CODE.
// This Word represents the empty word, a string of length of zero.
//
// returns: Initialized WordTable.
//
WordTable *wt_create(void){
    WordTable *wt = (WordTable *)malloc(MAX_CODE * sizeof(WordTable));
    if(!wt){
        return NULL;
    }
    memset(wt, 0, MAX_CODE * sizeof(WordTable));
    wt[EMPTY_CODE] = word_create(NULL, 0);      // Initialized with a single Word at index EMPTY_CODE.
    return wt;
}

//
// Resets a WordTable to having just the empty Word.
//
// wt:      WordTable to reset.
// returns: Void.
//
void wt_reset(WordTable *wt){
    if(wt){
        for(int i = START_CODE; i < MAX_CODE; i++){
            if(wt[i]){         
                word_delete(wt[i]);
                wt[i] = NULL;
            }
        }
    }
}

//
// Deletes an entire WordTable.
// All Words in the WordTable must be deleted as well.
//
// wt:      WordTable to free memory for.
// returns: Void.
//
void wt_delete(WordTable *wt){
    if(wt != NULL){
        wt_reset(wt);
        word_delete(wt[EMPTY_CODE]);
        free(wt);
    }
}

