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

static method_info *find_method(ClassFile *cf, const char *name);
static void JInstance_init(JInstance *instance, String name);



void vmThread_init(VMThread *vmt) {
    vmt->frame_count = 0;
}

void callFrame_init(CallFrame *cf, method_info *method) {
    cf->ip = method->byteCode->code;
    cf->code_length = method->byteCode->code_length;

    cf->max_locals = method->byteCode->max_locals;
    cf->locals = malloc(sizeof(Value) * cf->max_locals);

    cf->max_stack = method->byteCode->max_stack;
    cf->sp = malloc(sizeof(Value) * cf->max_stack);

    cf->method = method;
    cf->class = method->class;

    cf->constant_pool_count = method->class->constant_pool_count;
    cf->constant_pool = method->class->constant_pool;
}

void push_stack(CallFrame *cf, Value v) {
    *(cf->sp++) = v;
}

Value stack_pop(CallFrame *cf) {
    return *(--cf->sp);
}

Value stack_top(CallFrame *cf) {
    return *(cf->sp-1);
}

CallFrame *frame_top(VMThread *vmt) {
    return &vmt->frames[vmt->frame_count-1];
}

CallFrame *frame_pop(VMThread *vmt) {
    CallFrame *frame = &vmt->frames[vmt->frame_count-1];
    vmt->frame_count--;
    return frame;
}

u2 bytecode_read_u2(CallFrame *cf) {
    u2 v = (*cf->ip << 8) | *(cf->ip+1);
    cf->ip += 2;
    return v;
}

u1 bytecode_read_u1(CallFrame *cf) {
    return *cf->ip++;
}

Value execute_method(VMThread *vmt, method_info *method, Value *args, int nargs) {
    if (vmt->frame_count >= MAX_FRAMES) {
        printf("Exceeded max frames");
        exit(1);
    }
    CallFrame *cf = &vmt->frames[vmt->frame_count++];
    callFrame_init(cf, method);
    for (int i = 0; i < nargs; i++) {
        cf->locals[i] = args[i];
    }


    while (1) {
        switch(bytecode_read_u1(cf)) {
            case ICONST_0:
                push_stack(cf, INTVAL(0));
                break;
            case ICONST_1:
                push_stack(cf, INTVAL(1));
                break;
            case ICONST_2:
                push_stack(cf, INTVAL(2));
                break;
            case ICONST_3:
                push_stack(cf, INTVAL(3));
                break;
            case ICONST_4:
                push_stack(cf, INTVAL(4));
                break;
            case ICONST_5:
                push_stack(cf, INTVAL(5));
                break;
            case ISTORE_0:
                cf->locals[0] = stack_pop(cf);
                break;
            case ISTORE_1:
                cf->locals[1] = stack_pop(cf);
                break;
            case ISTORE_2:
                cf->locals[2] = stack_pop(cf);
                break;
            case ISTORE_3:
                cf->locals[3] = stack_pop(cf);
                break;
            case ILOAD_0:
                push_stack(cf, (cf->locals[0]));
                break;
            case ILOAD_1:
                push_stack(cf, (cf->locals[1]));
                break;
            case ILOAD_2:
                push_stack(cf, (cf->locals[2]));
                break;
            case ILOAD_3:
                push_stack(cf, (cf->locals[3]));
                break;
            case IADD: {
                int i1 = stack_pop(cf).i;
                int i2 = stack_pop(cf).i;
                push_stack(cf, INTVAL(i1+i2));
                break;
            }
            case INVOKESTATIC: {
                u2 i = bytecode_read_u2(cf);
                CONSTANT_MethodRef_info methodRef_info = cf->constant_pool[i].methodRef_info;
                CONSTANT_NameAndType_info nameAndType_info = cf->constant_pool[methodRef_info.name_and_type_index].nameAndType_info;
                String name = get_string_from_constant_pool(cf->class, nameAndType_info.name_index);
                method_info *method = find_method(cf->class, name.data);

                Value args[2];
                args[1] = stack_pop(cf);
                args[0] = stack_pop(cf);
                Value retVal = execute_method(vmt, method, args, 2);
                frame_pop(vmt);
                push_stack(cf, retVal);
                break;
            }
            case NEW: {
                u2 class_index = bytecode_read_u2(cf);
                CONSTANT_Class_info class_info = cf->constant_pool[class_index].class_info;
                String class_name = get_string_from_constant_pool(cf->class, class_info.name_index);
                JInstance *instance = malloc(sizeof(*instance));
                JInstance_init(instance, class_name);
                push_stack(cf, PTRVAL(instance));
                break;
            }
            case DUP:
                push_stack(cf, stack_top(cf));
                break;
            case INVOKESPECIAL: {
                u2 methodref_index = bytecode_read_u2(cf);
                break;

                CONSTANT_MethodRef_info methodRefInfo = cf->constant_pool[methodref_index].methodRef_info;

                CONSTANT_Class_info classInfo = cf->constant_pool[methodRefInfo.class_index].class_info;
                String class_name = get_string_from_constant_pool(cf, classInfo.name_index);

                CONSTANT_Class_info thisClassInfo = cf->constant_pool[cf->class->this_class].class_info;
                String this_class_name = get_string_from_constant_pool(cf, thisClassInfo.name_index);

                CONSTANT_NameAndType_info nameAndTypeInfo = cf->constant_pool[methodRefInfo.name_and_type_index].nameAndType_info;
                String method_name = get_string_from_constant_pool(cf, nameAndTypeInfo.name_index);

                if (str_compare(&class_name, &this_class_name) != 0)
                    break; //object <init> skip

                method_info *method = find_method(cf, method_name.data);
                execute_method(vmt, method, NULL, 0);
                frame_pop(vmt);
                break;
            }
            case ASTORE_1:
                cf->locals[1] = stack_pop(cf);
                break;
            case IRETURN: {
                Value val = stack_pop(cf);
                if (val.tag != INT) {
                    printf("Wrong type");
                    exit(1);
                }
                return val;
            }
            case RETURN:
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

void JInstance_init(JInstance *instance, String name) {
    instance->class_name = name;
}


int execute_class(VMThread *vmt, ClassFile *class) {
    method_info *main = find_method(class, "main");
    if (main == NULL) {
        printf("main class not found");
        return -1;
    }

    execute_method(vmt, main, NULL, 0);

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

int test_execute_class(VMThread *vmt, const char *path) {
    ByteBuf buf;
    read_file(path, &buf);
    ClassFile class;
    read_class(&buf, &class);

    execute_class(vmt, &class);
}

int test_execute_basic_class() {
    VMThread vmt;
    vmThread_init(&vmt);
    test_execute_class(&vmt, "./test/data/Basic.class");
}

int test_execute_add_class() {
    VMThread vmt;
    vmThread_init(&vmt);
    test_execute_class(&vmt, "./test/data/Add.class");
    ASSERT_EQ(2, frame_top(&vmt)->locals[3].i);
}

int test_execute_methodCall_class() {
    VMThread vmt;
    vmThread_init(&vmt);
    test_execute_class(&vmt, "./test/data/MethodCall.class");
    ASSERT_EQ(3, frame_top(&vmt)->locals[1].i);
}

int test_execute_instance_class() {
//    VMThread vmt;
    VMThread *vmt = malloc(sizeof(*vmt));
    vmThread_init(vmt);
    test_execute_class(vmt, "./test/data/Instance.class");

    ASSERT_EQ(PTR, frame_top(vmt)->locals[1].tag);
    String exp_classname = str_from_literal("Instance");
    String act_classname = ((JInstance *) frame_top(vmt)->locals[1].ptr)->class_name;
    ASSERT_EQ(0, str_compare(&exp_classname, &act_classname));
}



int main() {
//    test_basic_class();
//    test_execute_add_class();
//    test_execute_methodCall_class();
    test_execute_instance_class();
//    ByteBuf buf;
//    read_file("./test/data/Long.class", &buf);
//    ClassFile class;
//    read_class(&buf, &class);
}



