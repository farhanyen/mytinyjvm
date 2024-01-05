#pragma once
#include <stdlib.h>

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

typedef int8_t s1;
typedef int16_t s2;
typedef int32_t s4;


enum Type {
    BYTE,
    CHAR,
    INT,
    SHORT,
    FLOAT,
    DOUBLE,
    VOID,
    PTR
};

typedef struct Value {
    enum Type tag;
    union {
        u1 b;
        u2 c;
        short s;
        int i;
        float f;
        double d;
        void *ptr;
    };
} Value;

#define INTVAL(v) (Value) {.tag = INT, .i = v}
#define PTRVAL(v) (Value) {.tag = PTR, .ptr = v}
#define VOIDVAL() (Value) {.tag = VOID}


