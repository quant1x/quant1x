cmake_minimum_required(VERSION 3.20)
project(csv2_quote_test CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(csv2 CONFIG REQUIRED)

add_executable(csv2_quote_test test.cpp)
if(MSVC)
    target_compile_options(csv2_quote_test PRIVATE /utf-8)
endif()
target_link_libraries(csv2_quote_test PRIVATE csv2::csv2)
