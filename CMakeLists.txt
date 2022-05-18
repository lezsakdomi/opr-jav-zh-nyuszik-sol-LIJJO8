cmake_minimum_required(VERSION 3.22)
project(opr_zh C)

set(CMAKE_C_STANDARD 11)

add_executable(opr_zh
        main.c)

target_link_libraries(opr_zh rt)
