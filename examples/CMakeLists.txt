add_executable(parse_yaml_example parse_yaml_example.cpp)
# include project root so headers like ../quant1x/encoding/yaml.h resolve
target_include_directories(parse_yaml_example PRIVATE quant1x)
target_link_libraries(parse_yaml_example PRIVATE quant1x)


