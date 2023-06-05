cmake_minimum_required(VERSION 3.0.0)
set(PROJECT_NAME DB)

project(${PROJECT_NAME} VERSION 0.1.0)

set(SRC_FILES main_btree.cpp page.cpp page.hpp slot_header.cpp slot_header.hpp btree.cpp btree.hpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_compile_options(${PROJECT_NAME} PUBLIC -std=c++17 -pedantic-errors)