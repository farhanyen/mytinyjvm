#include "hashtable.h"
#include <stdlib.h>
#include <stdio.h>

int key_cmp(String *k1, String *k2) {
    return (k1->hash == k2->hash) && str_compare(k1, k2);
}

HashTableValue ht_get (HashTable *ht, String *key) {
    int i = key->hash & (ht->size - 1);
    HashEntry *cur = &ht->items[i];
    while (cur != NULL) {
        if (!key_cmp(cur->key, key))
            return cur->value;
        cur = cur->next;
    }
    return NULL;
}

void __ht_put(HashTable *ht, String *key, HashTableValue value) {
    ht->count++;

    int i = key->hash & (ht->size -1);
    HashEntry *cur = &ht->items[i];
    if (cur->key == NULL) {
        cur->key = key;
        cur->value = value;
        cur->next = NULL;
        return;
    }

    while (cur->next != NULL) {
        cur = cur->next;
    }
    HashEntry *new = malloc(sizeof(HashEntry));
    new->key = key;
    new->value = value;
    new->next = NULL;
    cur->next = new;
}

void ht_resize(HashTable *ht, int s) {
    HashEntry *old_items = ht->items;
    int old_size = ht->size;
    ht->items = malloc(sizeof(ht->items[0]) * s);
    ht->size = s;

    for (int i = 0; i < old_size; i++) {
        HashEntry *cur = &old_items;
        while (cur != NULL) {
            __ht_put(ht, cur->key, cur->value);
            cur = cur->next;
        }
    }

    free(old_items);
}

void ht_put(HashTable *ht, String *key, HashTableValue value) {
    if (ht->count * 2 >= ht->size)
        ht_resize(ht, ht->size *2);
    __ht_put(ht, key, value);
}


void ht_init(HashTable *ht) {
    ht->count = 0;
    ht->size = 0;
    ht->items = NULL;
    ht_resize(ht, HASHTABLE_INIT_SIZE);
}

//int main() {
//    HashTable *ht = malloc(sizeof(HashTable));
//    ht_init(ht);
//
//    String *s1 = malloc(sizeof(String));
//    char str1[] = "test1";
//    str_init(s1, str1, 5);
//    int v1 = 5;
//    ht_put(ht, s1, &v1);
//    int *p1 = ht_get(ht, s1);
//    printf("%d\n", *p1);
//
//    String *s2 = malloc(sizeof(String));
//    char str2[] = "test2";
//    str_init(s2, str2, 5);
//    int v2 = 6;
//    ht_put(ht, s2, &v2);
//    int *p2 = ht_get(ht, s2);
//    printf("%d\n", *p2);
//}