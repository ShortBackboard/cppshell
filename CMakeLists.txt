cmake_minimum_required(VERSION 3.22)

project(cppshell LANGUAGES CXX)

add_executable( cppshell )

target_sources( cppshell 
    PRIVATE
        main.cpp
    PUBLIC
)

target_compile_features( cppshell PRIVATE cxx_std_20 )



