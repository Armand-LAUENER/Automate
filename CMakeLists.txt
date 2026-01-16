cmake_minimum_required(VERSION 3.27)
project(Automate C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Automate
        Automate.c
        Automate.h
        main.c)
