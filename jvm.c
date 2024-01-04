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
#include <assert.h>

static method_info *find_method(ClassFile *cf, const char *name);
static void JInstance_init(JInstance *instance, String name);
static Value execute_method(VMThread *vmt, CallFrame *cf, method_info *method, Value* args, int nargs);


int get_no_of_args(String method_descriptor) {
    char *s = method_descriptor.data;
    int n = method_descriptor.len;

    int c = 0;

    for (int i = 1; i < n; i++) {
        switch(s[i]) {
            case ')':
                return c;
            case '[':
                break;
            case 'L': {
                c++;
                while(s[i] != ';')
                    i++;
                break;
            }
            default:
                c++;
        }
    }
    assert(1==0); //should never
    return -1;
}

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

void stack_push(CallFrame *cf, Value v) {
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

Value execute_static_method(VMThread *vmt, method_info *method, Value *args, int nargs) {
    assert(vmt->frame_count <= MAX_FRAMES);
    CallFrame *cf = &vmt->frames[vmt->frame_count++];
    callFrame_init(cf, method);
    for (int i = 0; i < nargs; i++) {
        cf->locals[i] = args[i];
    }
    Value v = execute_method(vmt, cf, method, args, nargs);
    frame_pop(vmt);
    return v;
}

Value execute_virtual_method(VMThread *vmt, method_info *method, JInstance *this, Value *args, int nargs) {
    assert(vmt->frame_count <= MAX_FRAMES);
    CallFrame *cf = &vmt->frames[vmt->frame_count++];
    callFrame_init(cf, method);

    cf->locals[0] = PTRVAL(this);
    for (int i = 0; i < nargs; i++) {
        cf->locals[i+1] = args[i];
    }
    Value v = execute_method(vmt, cf, method, args, nargs);
    frame_pop(vmt);
    return v;
}

Value execute_method(VMThread *vmt, CallFrame *cf, method_info *method, Value *args, int nargs) {
    while (1) {
        switch(bytecode_read_u1(cf)) {
            case ICONST_0:
                stack_push(cf, INTVAL(0));
                break;
            case ICONST_1:
                stack_push(cf, INTVAL(1));
                break;
            case ICONST_2:
                stack_push(cf, INTVAL(2));
                break;
            case ICONST_3:
                stack_push(cf, INTVAL(3));
                break;
            case ICONST_4:
                stack_push(cf, INTVAL(4));
                break;
            case ICONST_5:
                stack_push(cf, INTVAL(5));
                break;
            case ISTORE_0:
                cf->locals[0] = stack_pop(cf);
                break;
            case ISTORE_1:
            case ASTORE_1:
                cf->locals[1] = stack_pop(cf);
                break;
            case ISTORE_2:
                cf->locals[2] = stack_pop(cf);
                break;
            case ISTORE_3:
                cf->locals[3] = stack_pop(cf);
                break;
            case ILOAD_0:
                stack_push(cf, (cf->locals[0]));
                break;
            case ILOAD_1:
            case ALOAD_1:
                stack_push(cf, (cf->locals[1]));
                break;
            case ILOAD_2:
                stack_push(cf, (cf->locals[2]));
                break;
            case ILOAD_3:
                stack_push(cf, (cf->locals[3]));
                break;
            case IADD: {
                int i1 = stack_pop(cf).i;
                int i2 = stack_pop(cf).i;
                stack_push(cf, INTVAL(i1 + i2));
                break;
            }
            case NEW: {
                u2 class_index = bytecode_read_u2(cf);
                CONSTANT_Class_info class_info = cf->constant_pool[class_index].class_info;
                String class_name = string_from_constant_pool(cf->class, class_info.name_index);
                JInstance *instance = malloc(sizeof(*instance));
                JInstance_init(instance, class_name);
                stack_push(cf, PTRVAL(instance));
                break;
            }
            case DUP:
                stack_push(cf, stack_top(cf));
                break;
            case INVOKEVIRTUAL: {
                u2 methodref_index = bytecode_read_u2(cf);

                CONSTANT_MethodRef_info methodRefInfo = cf->constant_pool[methodref_index].methodRef_info;

                String class_name = classname_from_constant_pool(cf->class, methodRefInfo.class_index);
                String this_class_name = classname_from_constant_pool(cf->class, cf->class->this_class);

                if (str_compare(&class_name, &this_class_name) != 0) {
                    break;
                } //object <init> skip


                CONSTANT_NameAndType_info nameAndTypeInfo = cf->constant_pool[methodRefInfo.name_and_type_index].nameAndType_info;
                String method_name = string_from_constant_pool(cf->class, nameAndTypeInfo.name_index);
                String method_descriptor = string_from_constant_pool(cf->class, nameAndTypeInfo.descriptor_index);

                int n = get_no_of_args(method_descriptor);
                Value args[n];
                for (int i = n-1; i >= 0; i--) {
                    args[i] = stack_pop(cf);
                };

                JInstance *this = stack_pop(cf).ptr;
                method_info *method = find_method(cf->class, method_name.data);
                Value retVal = execute_virtual_method(vmt, method, this, args, 2);
                stack_push(cf, retVal);
                break;
            }
            case INVOKESTATIC: {
                u2 i = bytecode_read_u2(cf);

                CONSTANT_MethodRef_info methodRef_info = cf->constant_pool[i].methodRef_info;
                CONSTANT_NameAndType_info nameAndTypeInfo = cf->constant_pool[methodRef_info.name_and_type_index].nameAndType_info;

                String method_name = string_from_constant_pool(cf->class, nameAndTypeInfo.name_index);
                String method_descriptor = string_from_constant_pool(cf->class, nameAndTypeInfo.descriptor_index);
                int n = get_no_of_args(method_descriptor);
                Value args[n];
                for (int i = n-1; i >= 0; i--) {
                    args[i] = stack_pop(cf);
                };

                method_info *method = find_method(cf->class, method_name.data);
                Value retVal = execute_static_method(vmt, method, args, 2);
                stack_push(cf, retVal);
                break;
            }
            case INVOKESPECIAL: {
                u2 methodref_index = bytecode_read_u2(cf);
                break;

                CONSTANT_MethodRef_info methodRefInfo = cf->constant_pool[methodref_index].methodRef_info;

                String class_name = classname_from_constant_pool(cf->class, methodRefInfo.class_index);
                String this_class_name = classname_from_constant_pool(cf->class, cf->class->this_class);

                CONSTANT_NameAndType_info nameAndTypeInfo = cf->constant_pool[methodRefInfo.name_and_type_index].nameAndType_info;
                String method_name = string_from_constant_pool(cf, nameAndTypeInfo.name_index);

                if (str_compare(&class_name, &this_class_name) != 0) {
                    break;
                } //object <init> skip

                method_info *method = find_method(cf, method_name.data);
//                execute_method(vmt, method, NULL, 0);
                break;
            }
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
        String n1 = string_from_constant_pool(cf, method->name_index);
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

    assert(vmt->frame_count <= MAX_FRAMES);
    CallFrame *cf = &vmt->frames[vmt->frame_count++];
    callFrame_init(cf, main);


    Value v = execute_method(vmt, cf, main, NULL, 0);

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
    VMThread vmt;
    vmThread_init(&vmt);
    test_execute_class(&vmt, "./test/data/Instance.class");

    ASSERT_EQ(PTR, frame_top(&vmt)->locals[1].tag);
    String exp_classname = str_from_literal("Instance");
    String act_classname = ((JInstance *) frame_top(&vmt)->locals[1].ptr)->class_name;
    ASSERT_EQ(0, str_compare(&exp_classname, &act_classname));
}

int test_execute_instanceMethod_class() {
    VMThread vmt;
    vmThread_init(&vmt);
    test_execute_class(&vmt, "./test/data/InstanceMethod.class");

    ASSERT_EQ(3, frame_top(&vmt)->locals[2].i);
}


int main() {
    test_basic_class();
    test_execute_add_class();
    test_execute_methodCall_class();
    test_execute_instance_class();
    test_execute_instanceMethod_class();
//    ByteBuf buf;
//    read_file("./test/data/Long.class", &buf);
//    ClassFile class;
//    read_class(&buf, &class);
}



