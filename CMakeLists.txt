cmake_minimum_required(VERSION 3.21)
project(can_tp C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(can_tp
        src/cantp.c
        src/cantp.h
        example/main.c)
