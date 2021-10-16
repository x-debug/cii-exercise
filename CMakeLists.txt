cmake_minimum_required(VERSION 3.20)
project(cii C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_BUILD_TYPE "DEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

include_directories(inc)
add_executable(cii main.c src/atom.c src/mem.c src/except.c src/util.c inc/util.h inc/debug.h src/debug.c src/list.c)
