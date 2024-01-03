#pragma  once

#include "types.h"
#include "buf.h"
#include "mystring.h"


typedef struct ClassFile ClassFile;
enum cp_tags {
    CONSTANT_Class = 7,
    CONSTANT_Fieldref = 9,
    CONSTANT_Methodref = 10,
    CONSTANT_InterfaceMethodref = 11,
    CONSTANT_String = 8,
    CONSTANT_Integer = 3,
    CONSTANT_Float = 4,
    CONSTANT_Long = 5,
    CONSTANT_Double = 6,
    CONSTANT_NameAndType = 12,
    CONSTANT_Utf8 = 1,
    CONSTANT_MethodHandle = 15,
    CONSTANT_MethodType = 16,
    CONSTANT_InvokeDynamic = 18
};

typedef struct CONSTANT_Class_info {
    u2 name_index;
} CONSTANT_Class_info;

typedef struct CONSTANT_FieldRef_info {
    u2 class_index;
    u2 name_and_type_index;
} CONSTANT_FieldRef_info;

typedef struct CONSTANT_MethodRef_info {
    u2 class_index;
    u2 name_and_type_index;
} CONSTANT_MethodRef_info;

typedef struct CONSTANT_InterfaceMethodRef_info {
    u2 class_index;
    u2 name_and_type_index;
} CONSTANT_InterfaceMethodRef_info;

typedef struct CONSTANT_String_info {
    u2 string_index;
} CONSTANT_String_info;

typedef struct CONSTANT_Integer_info {
    u4 bytes;
} CONSTANT_Integer_info;

typedef struct CONSTANT_Float_info {
    u4 bytes;
} CONSTANT_Float_info;

typedef struct CONSTANT_Long_High_info {
    u4 bytes;
} CONSTANT_Long_High_info;

typedef struct CONSTANT_Long_Low_info {
    u4 bytes;
} CONSTANT_Long_Low_info;

typedef struct CONSTANT_Double_High_info {
    u4 bytes;
} CONSTANT_Double_High_info;

typedef struct CONSTANT_Double_Low_info {
    u4 bytes;
} CONSTANT_Double_Low_info;

typedef struct CONSTANT_NameAndType_info {
    u2 name_index;
    u2 descriptor_index;
} CONSTANT_NameAndType_info;

typedef struct CONSTANT_Utf8_info {
    u2 length;
    u1 *bytes;
} CONSTANT_Utf8_info;

typedef struct CONSTANT_MethodHandle_info {
    u1 reference_kind;
    u2 reference_index;
} CONSTANT_MethodHandle_info;

typedef struct CONSTANT_MethodType_info {
    u2 descriptor_index;
} CONSTANT_MethodType_info;

typedef struct CONSTANT_InvokeDynamic_info {
    u2 bootstrap_method_attr_index;
    u2 name_and_type_index;
} CONSTANT_InvokeDynamic_info;

typedef struct CONSTANT_value {
    u4 v;
} CONSTANT_value;

typedef struct CONSTANT_index_info {
    u2 index;
} CONSTANT_index_info;

typedef struct CONSTANT_kind_index_info {
    u1 kind;
    u2 index;
} CONSTANT_kind_index_info;

typedef struct CONSTANT_double_index_info {
    u2 index1;
    u2 index2;
} CONSTANT_double_index_info;

typedef struct CONSTANT_utf8_info {
    u2 length;
    u1 *bytes;
} CONSTANT_utf8_info;

typedef struct cp_info {
    u1 tag;
    union {
        CONSTANT_Class_info class_info;
        CONSTANT_FieldRef_info fieldRef_info;
        CONSTANT_MethodRef_info methodRef_info;
        CONSTANT_InterfaceMethodRef_info interfaceMethodRef_info;
        CONSTANT_String_info string_info;
        CONSTANT_Integer_info integer_info;
        CONSTANT_Float_info float_info;
        CONSTANT_Double_High_info doubleHigh_info;
        CONSTANT_Double_Low_info doubleLow_info;
        CONSTANT_Long_High_info longHigh_info;
        CONSTANT_Long_Low_info longLow_info;
        CONSTANT_NameAndType_info nameAndType_info;
        CONSTANT_Utf8_info utf8_info;
        CONSTANT_MethodHandle_info methodHandle_info;
        CONSTANT_MethodType_info methodType_info;
        CONSTANT_InvokeDynamic_info invokeDynamic_info;
    };
} cp_info;

enum access_flag {
    ACC_PUBLIC = 0x0001,
    ACC_PRIVATE = 0x0002,
    ACC_PROTECTED = 0x0004,
    ACC_STATIC = 0x0008,
    ACC_FINAL = 0x0010,
    ACC_VOLATILE = 0x0040,
    ACC_TRANSIENT = 0x0080,
    ACC_SYNTHETIC = 0x1000,
    ACC_ENUM = 0x4000,
    ACC_SYNCHRONIZED = 0x0020,
    ACC_BRIDGE = 0x0040,
    ACC_VARARGS = 0x0080,
    ACC_NATIVE = 0x0100,
    ACC_ABSTRACT = 0x0400,
    ACC_STRICT = 0x0800,
};

typedef struct attribute_info {
    u2 attribute_name_index;
    u4 attribute_length;
    u1 *info; //[attribute_length];
} attribute_info;

typedef struct field_info {
    u2             access_flags;
    u2             name_index;
    u2             descriptor_index;
    u2             attributes_count;
    attribute_info *attributes;
} field_info;

typedef struct exception_table {
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;
} exception_table;

typedef struct ByteCode {
    u2 max_stack;
    u2 max_locals;

    u4 instr_idx;
    u4 code_length;
    u1 *code;

    u2 exception_table_length;
    exception_table* exception_tables;
    u2 attributes_count;
    attribute_info *attributes;
} ByteCode;

typedef struct method_info {
    u2             access_flags;
    u2             name_index;
    u2             descriptor_index;
    u2             attributes_count;
    attribute_info *attributes; //[attributes_count];

    ClassFile *class;
    ByteCode *byteCode;
    String name;
} method_info;

typedef struct ClassFile {
    u2             minor_version;
    u2             major_version;
    u2             constant_pool_count;
    cp_info        *constant_pool; //const_pool_count-1
    u2             access_flags;
    u2             this_class;
    u2             super_class;
    u2             interfaces_count;
    u2             *interfaces;
    u2             fields_count;
    field_info     *fields;
    u2             methods_count;
    method_info    *methods;
    u2             attributes_count;
    attribute_info *attributes;
} ClassFile;


String get_string_from_constant_pool(ClassFile *cf, int name_index);

int read_class (ByteBuf *buf, ClassFile *cf);