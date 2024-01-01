//
// Created by Farhan Zain on 19/12/2023.
//

#include "util.h"

uint32_t fnv_32_hash(char *s, int len) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < len; i++) {
        hash ^= (unsigned char) s[i];
        hash *= 16777619u;
    }
    return hash;
}