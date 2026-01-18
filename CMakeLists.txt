cmake_minimum_required(VERSION 3.10)
project(Automate C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra)
endif()

add_executable(Automate
        main.c
        AutomateCore.c
        AutomateCore.h
        AutomateIO.c
        AutomateIO.h
        AutomateAnalysis.c
        AutomateAnalysis.h
        AutomateTransform.c
        AutomateTransform.h
)