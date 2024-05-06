cmake_minimum_required(VERSION 3.21)
project(doko_mc_ai LANGUAGES C CXX VERSION 0.1)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(Boost 1.80.0 REQUIRED COMPONENTS system json)

add_subdirectory(protocol)
add_subdirectory(src)
