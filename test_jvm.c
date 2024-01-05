#include "jvm.h"
#include "util.h"
#include "test.h"

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


int test_execute_instanceFields_class() {
    VMThread vmt;
    vmThread_init(&vmt);
    test_execute_class(&vmt, "./test/data/InstanceFields.class");

    ASSERT_EQ(5, frame_top(&vmt)->locals[2].i);
}


int main() {
    test_execute_add_class();
    test_execute_methodCall_class();
    test_execute_instance_class();
    test_execute_instanceMethod_class();
    test_execute_instanceFields_class();
}