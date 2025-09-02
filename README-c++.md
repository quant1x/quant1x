# q1x

#### 介绍
c++版本的量化基础库

### 1. 依赖库安装
#### 1.1 protobuf 3.21.11
protobuf 从3.21.12起引用google的abseil库, 非常复杂, 不适用, 没有必要使用太高的版本
- 下载地址

```shell
wget https://gh-proxy.com/github.com/protocolbuffers/protobuf/releases/download/v21.11/protobuf-cpp-3.21.11.zip
```
- gcc/clang编译
```shell
cmake -DCMAKE_INSTALL_PREFIX=~/runtime -Dprotobuf_BUILD_TESTS=OFF -G "Unix Makefiles" ../
```
- msvc编译
```shell
cmake -DCMAKE_INSTALL_PREFIX=d:/dev -G "Visual Studio 17 2022" -A x64 ..
```
### 1.2 其它依赖库
```shell
vcpkg install yaml-cpp zlib asio xtensor mimalloc spdlog fmt duktape benchmark catch2 flatbuffers capnproto
```

- 安装
```shell
cmake -DCMAKE_INSTALL_PREFIX=d:/runtime -Dprotobuf_BUILD_TESTS=OFF -G "MinGW Makefiles" ../ 
```

### 2. 编译
```shell
cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S .  -B cmake-build-debug
cmake --build cmake-build-debug --target test-crash -j 18
cmake --build cmake-build-debug --verbose --target q2x -j 18
```

#### 2.2 编译主程序
```shell
cmake --build cmake-build-debug --verbose --target q2x -j 18
```
#### 2.3 安装主程序
- 默认安装路径 ~/runtime/bin
```shell
ninja -C cmake-build-debug install
```

### 3. 测试
```shell
ctest --test-dir cmake-build-debug --output-on-failure
```

### 4. 运行
```shell
./cmake-build-debug/bin/q2x --help
```

### 5. 关于开发工具
#### 5.1 Visual Studio 2022
- Visual Studio 2022 需要安装 C++ CMake 工具, 以及 C++ CMake 工具集
- 配置vcpkg.json, MSVC需要使用静态链接库, 需要安装对应的vcpkg triplet
```shell
vcpkg install --triplet x64-windows-static
```
#### 5.2 CLion
CLion 需要安装 CMake, CMake 需要安装 Ninja, CLion 需要安装 C++ 插件
