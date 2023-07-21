cmake_minimum_required(VERSION 3.25)
project(elfparser)

set(CMAKE_CXX_STANDARD 17)

add_executable(elfparser main.cpp elf.h Elf32Parser.cpp Elf32Parser.h)
