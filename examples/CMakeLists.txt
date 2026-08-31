add_executable(parse_yaml_example parse_yaml_example.cpp)
# include project root so headers like <quant1x/encoding/yaml.h> resolve.
# 注: 该 target 的头文件路径依赖链接 quant1x 传递过来的 PUBLIC include 目录
# (根 CMakeLists 中的 $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>), 无需在此重复指定.
target_link_libraries(parse_yaml_example PRIVATE quant1x)

# Vyukov MPMC 队列演示: 同时作为 CTest 用例运行,
# 失败 (生产/消费数量不一致) 时以非 0 退出码结束.
add_executable(vyukov_demo vyukov_demo.cpp)
target_link_libraries(vyukov_demo PRIVATE quant1x)
add_test(NAME vyukov_demo COMMAND vyukov_demo)
