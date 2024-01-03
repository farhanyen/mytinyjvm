#include "buf.h"
#include <string.h>

//slice of existing data not responsible for freeing
void buf_create(ByteBuf *buf, uint8_t *data, int size) {
    buf->data = data;
    buf->size = size;
    buf->off = 0;
}

u1 buf_read_u1 (ByteBuf *buf) {
    u1 v = buf->data[buf->off];
    buf->off++;
    return v;
}

u2 buf_read_u2 (ByteBuf *buf) {
    u4 v = buf->data[buf->off] << 8 |
           buf->data[buf->off+1];
    buf->off += 2;
    return v;
}


u4 buf_read_u4 (ByteBuf *buf) {
    u4 v = buf->data[buf->off] << 24 |
        buf->data[buf->off+1] << 16 |
        buf->data[buf->off+2] << 8 |
        buf->data[buf->off+3];
    buf->off += 4;
    return v;
}
