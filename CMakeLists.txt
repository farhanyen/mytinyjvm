cmake_minimum_required(VERSION 3.26)
project(mytinyjvm C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")

include_directories(.)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


add_executable(test_jvm
        hashtable.c
        hashtable.h mystring.c mystring.h util.c util.h jvm.c jvm.h types.c types.h buf.c buf.h classloader.c classloader.h test_jvm.c test.h)

add_executable(test_classloader
        hashtable.c
        hashtable.h mystring.c mystring.h util.c util.h jvm.c jvm.h types.c types.h buf.c buf.h classloader.c classloader.h test_classloader.c test.h)

add_executable(test test.c)