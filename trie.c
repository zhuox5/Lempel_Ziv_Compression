#include "trie.h"
#include "code.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <inttypes.h>
//
// Constructor for a TrieNode.
//
// index:   Index of the constructed TrieNode.
// returns: Pointer to a TrieNode that has been allocated memory.
//
TrieNode *trie_node_create(uint16_t index){

    TrieNode* tr = (TrieNode *)malloc(sizeof(TrieNode));
    if(!tr){
        return NULL;
    }
    memset(tr, 0, sizeof(TrieNode));
    tr->code = index;           //set code

    return tr;
}

//
// Destructor for a TrieNode.
//
// n:       TrieNode to free allocated memory for.
// returns: Void.
//
void trie_node_delete(TrieNode *n){
    if(n){
        free(n);
        //n->code = 0;    //Do i need this?
    }
    return;
}

//
// Initializes a Trie: a root TrieNode with the index EMPTY_CODE.
//
// returns: Pointer to the root of a Trie.
//
TrieNode *trie_create(void){
    TrieNode *n = trie_node_create(EMPTY_CODE);
    if(n){
       return n;
    }
    return NULL;
}

//
// Resets a Trie to just the root TrieNode.
//
// root:    Root of the Trie to reset.
// returns: Void.
//
void trie_reset(TrieNode *root){
    if(root){
        for(int i = 0; i < ALPHABET; i++){
            trie_delete(root->children[i]);     //late check if NULL
            root->children[i] = NULL;
        }
    }
    return;
}

//
// Deletes a sub-Trie starting from the sub-Trie's root.
//
// n:       Root of the sub-Trie to delete.
// returns: Void.
//
void trie_delete(TrieNode *n){
    if(n){
        trie_reset(n);
        trie_node_delete(n);
    }
}

//
// Returns a pointer to the child TrieNode reprsenting the symbol sym.
// If the symbol doesn't exist, NULL is returned.
//
// n:       TrieNode to step from.
// sym:     Symbol to check for.
// returns: Pointer to the TrieNode representing the symbol.
//
TrieNode *trie_step(TrieNode *n, uint8_t sym){
    TrieNode *temp = n->children[sym];
    if(temp){
        return temp;                        // need to check NULL
    }
    return NULL;                        
}
