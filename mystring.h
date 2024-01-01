#pragma once

#include <stdlib.h>

typedef struct {
    uint32_t hash;
    int len;
    char *data;
} String;


void str_init(String *str, const char *data, int len);

int str_compare(String *s1, String *s2);

String str_from_literal(const char *s);
