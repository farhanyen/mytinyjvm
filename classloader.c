//
// Created by Farhan Zain on 27/12/2023.
//
#include <stdio.h>
#include "classloader.h"

String classname_from_constant_pool(ClassFile *cf, int class_index) {
    CONSTANT_Class_info classInfo = cf->constant_pool[class_index].class_info;
    return string_from_constant_pool(cf, classInfo.name_index);
}

String string_from_constant_pool(ClassFile *cf, int name_index) {
    CONSTANT_Utf8_info utf8_info = cf->constant_pool[name_index].utf8_info;
    String str;
    str_init(&str, utf8_info.bytes, utf8_info.length);
    return str;
}

void parse_constant(ByteBuf *buf, cp_info *con) {
    u1 tag = buf_read_u1(buf);

    switch(tag) {
        case CONSTANT_Class:
            con->class_info.name_index = buf_read_u2(buf);
            break;
        case CONSTANT_Fieldref:
            con->fieldRef_info.class_index = buf_read_u2(buf);
            con->fieldRef_info.name_and_type_index = buf_read_u2(buf);
            break;
        case CONSTANT_Methodref :
            con->methodRef_info.class_index = buf_read_u2(buf);
            con->methodRef_info.name_and_type_index = buf_read_u2(buf);
            break;
        case CONSTANT_InterfaceMethodref:
            con->interfaceMethodRef_info.class_index = buf_read_u2(buf);
            con->interfaceMethodRef_info.name_and_type_index = buf_read_u2(buf);
            break;
        case CONSTANT_String:
            con->string_info.string_index = buf_read_u2(buf);
            break;
        case CONSTANT_Integer:
            con->integer_info.bytes = buf_read_u4(buf);
            break;
        case CONSTANT_Float:
            con->float_info.bytes = buf_read_u4(buf);
            break;
        case CONSTANT_Long:
            con->longHigh_info.bytes = buf_read_u4(buf);
            break;
        case CONSTANT_Double:
            con->doubleHigh_info.bytes = buf_read_u4(buf);
            break;
        case CONSTANT_NameAndType:
            con->nameAndType_info.name_index = buf_read_u2(buf);
            con->nameAndType_info.descriptor_index = buf_read_u2(buf);
            break;
        case CONSTANT_Utf8:
            con->utf8_info.length = buf_read_u2(buf);
            con->utf8_info.bytes = malloc(con->utf8_info.length);
            for (int i = 0; i < con->utf8_info.length; i++) {
                con->utf8_info.bytes[i] = buf_read_u1(buf);
            }
            break;
        case CONSTANT_MethodHandle:
            con->methodHandle_info.reference_kind = buf_read_u1(buf);
            con->methodHandle_info.reference_index = buf_read_u2(buf);
            break;
        case CONSTANT_MethodType:
            con->methodType_info.descriptor_index = buf_read_u2(buf);
            break;
        case CONSTANT_InvokeDynamic:
            con->invokeDynamic_info.bootstrap_method_attr_index = buf_read_u2(buf);
            con->invokeDynamic_info.name_and_type_index = buf_read_u2(buf);
            break;
    }
}

void parse_attribute_info(ByteBuf *buf, attribute_info *attribute) {
    attribute->attribute_name_index = buf_read_u2(buf);
    attribute->attribute_length = buf_read_u4(buf);
    attribute->info = malloc(attribute->attribute_length);
    for (int i = 0; i < attribute->attribute_length; i++) {
        attribute->info[i] = buf_read_u1(buf);
    }
}

void parse_field_info(ByteBuf *buf, field_info *field) {
    field->access_flags = buf_read_u2(buf);
    field->name_index = buf_read_u2(buf);
    field->descriptor_index = buf_read_u2(buf);
    field->attributes_count = buf_read_u2(buf);
    field->attributes = malloc(sizeof(*field->attributes) * field->attributes_count);
    for (int i = 0; i < field->attributes_count; i++) {
        parse_attribute_info(buf, &field->attributes[i]);
    }
}

void parse_exception_table(ByteBuf *buf, exception_table *table) {
    table->start_pc = buf_read_u2(buf);
    table->end_pc = buf_read_u2(buf);
    table->handler_pc = buf_read_u2(buf);
    table->catch_type = buf_read_u2(buf);
}

void parse_method_bytecode(ByteBuf *buf, ByteCode *code) {
    code->max_stack = buf_read_u2(buf);
    code->max_locals = buf_read_u2(buf);
    code->code_length = buf_read_u4(buf);
    code->code = malloc(code->code_length);
    for (int i = 0; i < code->code_length; i++) {
        code->code[i] = buf_read_u1(buf);
    }
    code->exception_table_length = buf_read_u2(buf);
    code->exception_tables = malloc(sizeof(code->exception_tables[0]) * code->exception_table_length);
    for (int i = 0; i < code->exception_table_length; i++) {
        parse_exception_table(buf, &code->exception_tables[i]);
    }
    code->attributes_count = buf_read_u2(buf);
    code->attributes = malloc(sizeof(code->attributes[0]) * code->attributes_count);
    for (int i = 0; i < code->attributes_count; i++) {
        parse_attribute_info(buf, &code->attributes[i]);
    }
}

void parse_method_info(ByteBuf *buf, method_info *method, ClassFile *cf) {
    method->access_flags = buf_read_u2(buf);
    method->name_index = buf_read_u2(buf);
    method->descriptor_index = buf_read_u2(buf);
    method->attributes_count = buf_read_u2(buf);
    method->attributes = malloc(sizeof(method->attributes[0]) * method->attributes_count);
    for (int i = 0; i < method->attributes_count; i++) {
        parse_attribute_info(buf, &method->attributes[i]);
    }

    method->class = cf;
    method->name = string_from_constant_pool(cf, method->name_index);
    for (int i = 0; i < method->attributes_count; i++) {
        attribute_info attr = method->attributes[i];
        String attr_name = string_from_constant_pool(cf, attr.attribute_name_index);
        String code = str_from_literal("Code");
        if (str_compare(&attr_name, &code) == 0) {
            ByteBuf buf_code;
            buf_create(&buf_code, attr.info, attr.attribute_length);
            method->byteCode = malloc(sizeof (*method->byteCode));
            parse_method_bytecode(&buf_code, method->byteCode);
        }
    }
}


int read_class (ByteBuf *buf, ClassFile *cf) {
    u4 magic = buf_read_u4(buf);
    if (magic != 0xcafebabe) {
        printf("magic wrong %x", magic);
        return -1;
    }
    cf->minor_version = buf_read_u2(buf);
    cf->major_version = buf_read_u2(buf);
    cf->constant_pool_count = buf_read_u2(buf);
    cf->constant_pool = malloc(cf->constant_pool_count * sizeof(cp_info));
    for (int i = 1; i < cf->constant_pool_count;i++) {
        parse_constant(buf, &cf->constant_pool[i]);
        cp_info con;
        if (con.tag == CONSTANT_Double) {
            con.doubleLow_info.bytes = buf_read_u4(buf);
            i++;
            cf->constant_pool[i] = con;
        }
        if (con.tag == CONSTANT_Long) {
            con.longLow_info.bytes = buf_read_u4(buf);
            i++;
            cf->constant_pool[i] = con;
        }
    }

    cf->access_flags = buf_read_u2(buf);
    cf->this_class = buf_read_u2(buf);
    cf->super_class = buf_read_u2(buf);

    cf->interfaces_count = buf_read_u2(buf);
    cf->interfaces = malloc(cf->interfaces_count * sizeof(cf->interfaces[0]));
    for (int i = 0; i < cf->interfaces_count; i++) {
        cf->interfaces[i] = buf_read_u2(buf);
    }

    cf->fields_count = buf_read_u2(buf);
    cf->fields = malloc(cf->fields_count * sizeof(field_info));
    for (int i = 0; i < cf->fields_count;i++) {
        parse_field_info(buf, &cf->fields[i]);
    }

    cf->methods_count = buf_read_u2(buf);
    cf->methods = malloc(cf->methods_count * sizeof(method_info));
    for (int i = 0; i < cf->methods_count;i++) {
        parse_method_info(buf, &cf->methods[i], cf);
    }

    cf->attributes_count = buf_read_u2(buf);
    cf->attributes = malloc(cf->attributes_count * sizeof(attribute_info));
    for (int i = 0; i < cf->attributes_count;i++) {
        parse_attribute_info(buf, &cf->attributes[i]);
    }

    return 0;
}



