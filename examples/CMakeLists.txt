cmake_minimum_required(VERSION 3.15)
project(parse_yaml_example)

find_package(yaml-cpp CONFIG REQUIRED)

add_executable(parse_yaml_example parse_yaml_example.cpp)
# include project root so headers like ../quant1x/encoding/yaml.h resolve
target_include_directories(parse_yaml_example PRIVATE ${CMAKE_SOURCE_DIR}/..)
target_link_libraries(parse_yaml_example PRIVATE yaml-cpp::yaml-cpp)


