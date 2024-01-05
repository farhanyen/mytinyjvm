
#include "jvm.h"
#include "util.h"
#include "test.h"

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

int main() {
    test_basic_class();

    //    ByteBuf buf;
//    read_file("./test/data/Long.class", &buf);
//    ClassFile class;
//    read_class(&buf, &class);
}