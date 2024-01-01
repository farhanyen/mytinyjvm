#include "mystring.h"
#include "util.h"
#include <string.h>

void str_init(String *str, const char *data, int len) {
    str->data = data;
    str->hash = fnv_32_hash(data, len);
    str->len = len;
}

int str_compare(String *s1, String *s2) {
    if (s1->len != s2->len)
        return 1;
    return memcmp(s1->data, s2->data, s1->len);
}

String str_from_literal(const char *s) {
    String str;
    str_init(&str, s, strlen(s));
    return str;
}