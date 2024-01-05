#pragma once

#include <stdio.h>

#define ASSERT_EQ(exp, act)  \
    do {                      \
    /*num_tests++;*/             \
    if((exp) != (act)) {        \
        printf("TEST FAILED: %s\n\t %s:%d EXPECTED %s obtained %s\n", __FUNCTION__, __FILE_NAME__, __LINE__, #exp, #act); \
        /*failures++;*/ \
    }                         \
    }while(0)