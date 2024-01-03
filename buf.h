#pragma once
#include "types.h"


typedef struct ByteBuf {
    uint8_t *data;
    int size;
    int off;
} ByteBuf;

void buf_create(ByteBuf *buf, uint8_t *data, int size);
u1 buf_read_u1(ByteBuf *buf);
u2 buf_read_u2(ByteBuf *buf);
u4 buf_read_u4(ByteBuf *buf);