//
// Created by Farhan Zain on 26/12/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jvm.h"
#include "buf.h"
#include "classloader.h"
#include <mystring.h>



int callFrame_init(CallFrame *cf, method_info *method) {
    cf->ip = method->byteCode->code;
    cf->max_locals = method->byteCode->max_locals;
    cf->locals = malloc(sizeof(Value) * cf->max_locals);
    cf->max_stack = method->byteCode->max_stack;
    cf->sp = malloc(sizeof(Value) * cf->max_stack);
}

void push_stack(CallFrame *cf, Value v) {
    *(cf->sp++) = v;
}

Value pop_stack(CallFrame *cf) {
    return *(--cf->sp);
}

Value execute_method(method_info *method) {
    CallFrame callFrame;
    callFrame_init(&callFrame, method);
    CallFrame *cf = &callFrame;
    while (1) {
        switch(*cf->ip) {
            case ICONST_0:
                push_stack(cf, INTVAL(0));
                cf->ip++;
                break;
            case ICONST_1:
                push_stack(cf, INTVAL(1));
                cf->ip++;
                break;
            case ICONST_2:
                push_stack(cf, INTVAL(2));
                cf->ip++;
                break;
            case ICONST_3:
                push_stack(cf, INTVAL(3));
                cf->ip++;
                break;
            case ICONST_4:
                push_stack(cf, INTVAL(4));
                cf->ip++;
                break;
            case ICONST_5:
                push_stack(cf, INTVAL(5));
                cf->ip++;
                break;
            case ISTORE_0:
                cf->locals[0] = pop_stack(cf);
                cf->ip++;
                break;
            case ISTORE_1:
                cf->locals[1] = pop_stack(cf);
                cf->ip++;
                break;
            case ISTORE_2:
                cf->locals[2] = pop_stack(cf);
                cf->ip++;
                break;
            case ISTORE_3:
                cf->locals[3] = pop_stack(cf);
                cf->ip++;
                break;
            case ILOAD_0:
                push_stack(cf, (cf->locals[0]));
                cf->ip++;
                break;
            case ILOAD_1:
                push_stack(cf, (cf->locals[1]));
                cf->ip++;
                break;
            case ILOAD_2:
                push_stack(cf, (cf->locals[2]));
                cf->ip++;
                break;
            case ILOAD_3:
                push_stack(cf, (cf->locals[3]));
                cf->ip++;
                break;
            case IADD: {
                int i1 = pop_stack(cf).i;
                int i2 = pop_stack(cf).i;
                push_stack(cf, INTVAL(i1+i2));
                cf->ip++;
                break;
            }
            case RETURN:
                cf->ip++;
                return VOIDVAL();
        }
    }
}

method_info *find_method(ClassFile *cf, const char *name) {
    for (int i = 0; i < cf->methods_count; i++) {
        method_info *method = &cf->methods[i];
        String n1 = get_string_from_constant_pool(cf, method->name_index);
        String n2;
        str_init(&n2, name, strlen(name));
        if (str_compare(&n1, &n2) == 0) {
            return method;
        }
    }
    return NULL;
}


int execute_class(ClassFile *class) {
    method_info *main = find_method(class, "main");
    if (main == NULL) {
        printf("main class not found");
        return -1;
    }

    execute_method(main);

    return 0;
}

void bytebuf_init(ByteBuf *buf, uint8_t *data, int size) {
    buf->size = size;
    buf->data = data;
    buf->off = 0;
}

int read_file(const char *name, ByteBuf *buf) {
    FILE *fp = fopen(name, "rb");
    if (fp == NULL)
        return -1;
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);

    uint8_t *c = malloc(size);
    if (c == NULL)
        return -1;
    fread(c, size, 1, fp);
    fclose(fp);
    bytebuf_init(buf, c, size);
    return 0;
}

int test_basic_class() {
    ByteBuf buf;
    read_file("./test/data/Basic.class", &buf);
    ClassFile class;
    read_class(&buf, &class);

    ASSERT_EQ(27, class.constant_pool_count);
    ASSERT_EQ(21, class.constant_pool[15].class_info.name_index);

    ASSERT_EQ(0, class.interfaces_count);
    ASSERT_EQ(0, class.fields_count);
    ASSERT_EQ(2, class.methods_count);
    ASSERT_EQ(1, class.attributes_count);

    ASSERT_EQ(ACC_PUBLIC, class.methods[0].access_flags);
    ASSERT_EQ(7, class.methods[0].descriptor_index);
    ASSERT_EQ(6, class.methods[0].name_index);
    ASSERT_EQ(1, class.methods[0].attributes_count);
    // TODO need to test attributes

    ASSERT_EQ(ACC_PUBLIC | ACC_STATIC, class.methods[1].access_flags);
    ASSERT_EQ(11, class.methods[1].descriptor_index);
    ASSERT_EQ(10, class.methods[1].name_index);
    ASSERT_EQ(1, class.methods[1].attributes_count);
}

int test_execute_basic_class() {
    ByteBuf buf;
    read_file("./test/data/Basic.class", &buf);
    ClassFile class;
    read_class(&buf, &class);
    execute_class(&class);
}

int test_execute_add_class() {
    ByteBuf buf;
    read_file("./test/data/Add.class", &buf);
    ClassFile class;
    read_class(&buf, &class);
    execute_class(&class);
}

int main() {
//    test_basic_class();
    test_execute_add_class();
//    ByteBuf buf;
//    read_file("./test/data/Long.class", &buf);
//    ClassFile class;
//    read_class(&buf, &class);
}



