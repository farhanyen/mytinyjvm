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

static method_info *get_method(VMThread *vmt, String methodKey);
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
    ht_init(&vmt->ht);
}

void callFrame_init(CallFrame *cf, method_info *method) {
    cf->code = method->byteCode->code;
    cf->code_idx = 0;
    cf->code_length = method->byteCode->code_length;

    cf->max_locals = method->byteCode->max_locals;
    cf->locals = malloc(sizeof(Value) * cf->max_locals);

    cf->max_stack = method->byteCode->max_stack;
    cf->stack_idx = 0;
    cf->stack = malloc(sizeof(Value) * cf->max_stack);

    cf->method = method;
    cf->class = method->class;

    cf->constant_pool_count = method->class->constant_pool_count;
    cf->constant_pool = method->class->constant_pool;
}

void stack_push(CallFrame *cf, Value v) {
    cf->stack[cf->stack_idx++] = v;
}

Value stack_pop(CallFrame *cf) {
    return cf->stack[--cf->stack_idx];
}

Value stack_top(CallFrame *cf) {
    return cf->stack[cf->stack_idx-1];
}

CallFrame *frame_top(VMThread *vmt) {
    return &vmt->frames[vmt->frame_count-1];
}

CallFrame *frame_pop(VMThread *vmt) {
    CallFrame *frame = &vmt->frames[vmt->frame_count-1];
    vmt->frame_count--;
    return frame;
}

u1 bytecode_read_u1(CallFrame *cf) {
    return cf->code[cf->code_idx++];
}

u2 bytecode_read_u2(CallFrame *cf) {
    return bytecode_read_u1(cf) << 8 | bytecode_read_u1(cf);
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
    u1 code;
    while (1) {
        switch(code = bytecode_read_u1(cf)) {
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
            case ALOAD_0:
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
            case GETFIELD: {
                u2 fieldref_index = bytecode_read_u2(cf);
                JInstance *instance = stack_pop(cf).ptr;

                CONSTANT_FieldRef_info fieldRefInfo = cf->class->constant_pool[fieldref_index].fieldRef_info;

                u2 name_and_type_index = fieldRefInfo.name_and_type_index;
                CONSTANT_NameAndType_info nameAndTypeInfo = cf->class->constant_pool[name_and_type_index].nameAndType_info;

                String fieldName = string_from_constant_pool(cf->class, nameAndTypeInfo.name_index);
                String fieldDesc = string_from_constant_pool(cf->class, nameAndTypeInfo.descriptor_index);

                String *fieldKey = malloc(sizeof(*fieldKey));
                *fieldKey = str_from_member(instance->class_name, fieldName, fieldDesc);

                Value v = *(Value *)ht_get(&vmt->ht, fieldKey);
                stack_push(cf, v);
                break;
            }
            case PUTFIELD: {
                u2 fieldref_index = bytecode_read_u2(cf);

                CONSTANT_FieldRef_info fieldRefInfo = cf->class->constant_pool[fieldref_index].fieldRef_info;
                u2 nameAndType_index = fieldRefInfo.name_and_type_index;
                CONSTANT_NameAndType_info nameAndTypeInfo = cf->class->constant_pool[nameAndType_index].nameAndType_info;

                String fieldName = string_from_constant_pool(cf->class, nameAndTypeInfo.name_index);
                String fieldDesc = string_from_constant_pool(cf->class, nameAndTypeInfo.descriptor_index);

                Value v = stack_pop(cf);
                JInstance *instance = stack_pop(cf).ptr;

                String *fieldKey = malloc(sizeof(*fieldKey));
                *fieldKey = str_from_member(instance->class_name, fieldName, fieldDesc);

                Value *hashValue = malloc(sizeof(*hashValue));
                *hashValue = v;
                ht_put(&vmt->ht, fieldKey, hashValue);
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
                }


                CONSTANT_NameAndType_info nameAndTypeInfo = cf->constant_pool[methodRefInfo.name_and_type_index].nameAndType_info;
                String method_name = string_from_constant_pool(cf->class, nameAndTypeInfo.name_index);
                String method_descriptor = string_from_constant_pool(cf->class, nameAndTypeInfo.descriptor_index);

                int n = get_no_of_args(method_descriptor);
                Value args[n];
                for (int i = n-1; i >= 0; i--) {
                    args[i] = stack_pop(cf);
                };

                JInstance *this = stack_pop(cf).ptr;
                method_info *method = get_method(vmt, str_from_member(class_name, method_name, method_descriptor));
                Value retVal = execute_virtual_method(vmt, method, this, args, 2);
                stack_push(cf, retVal);
                break;
            }
            case INVOKESTATIC: {
                u2 i = bytecode_read_u2(cf);

                CONSTANT_MethodRef_info methodRef_info = cf->constant_pool[i].methodRef_info;
                String class_name = classname_from_constant_pool(cf->class, methodRef_info.class_index);
                CONSTANT_NameAndType_info nameAndTypeInfo = cf->constant_pool[methodRef_info.name_and_type_index].nameAndType_info;

                String method_name = string_from_constant_pool(cf->class, nameAndTypeInfo.name_index);
                String method_descriptor = string_from_constant_pool(cf->class, nameAndTypeInfo.descriptor_index);
                int n = get_no_of_args(method_descriptor);
                Value args[n];
                for (int i = n-1; i >= 0; i--) {
                    args[i] = stack_pop(cf);
                };

                String methodKey = str_from_member(class_name, method_name, method_descriptor);
                method_info *method = get_method(vmt, methodKey);
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
                String method_descriptor = string_from_constant_pool(cf->class, nameAndTypeInfo.descriptor_index);


                if (str_compare(&class_name, &this_class_name) != 0) {
                    break;
                } //object <init> skip

                method_info *method = get_method(vmt, str_from_member(class_name, method_name, method_descriptor));
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
            default:
                printf("%d not implemented", code);
                assert(1==0);
        }
    }
}



method_info *get_method(VMThread *vmt, String methodKey) {
    Value *v =  (Value *) ht_get(&vmt->ht, &methodKey);
    assert(v->tag == PTR);
    return v->ptr;
}

void JInstance_init(JInstance *instance, String name) {
    instance->class_name = name;
}


int execute_class(VMThread *vmt, ClassFile *class) {
    process_class(class, &vmt->ht);
    method_info *main = get_method(vmt, str_from_member(class->name, str_from_literal("main"), str_from_literal("([Ljava/lang/String;)V")));
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








