cmake_minimum_required(VERSION 3.10)
project(cat LANGUAGES C VERSION 0.0.3)

set(CMAKE_C_STANDARD 90)

add_executable(cat main.c cat.c cat.h platform.c platform.h)

install(TARGETS cat DESTINATION bin)
