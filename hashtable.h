#pragma once
#include "mystring.h"

#define HASHTABLE_INIT_SIZE 8

typedef void* HashTableValue;


typedef struct HashEntry {
    String *key;
    HashTableValue value;
    struct HashEntry *next;
} HashEntry;

typedef struct HashTable {
    int count;
    int size;
    HashEntry *items;
} HashTable;

void ht_init(HashTable *ht);

void ht_put(HashTable *ht, String *key, HashTableValue value);
HashTableValue ht_get(HashTable *ht, String *key);



