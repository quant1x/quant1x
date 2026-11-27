from conan import ConanFile


class Quant1XDependencies(ConanFile):
    """Quant1X C++ 第三方依赖（Conan 2 consumer 配方）.

    仅用于声明依赖并生成 conan_toolchain.cmake / CMakeDeps 配置，不产出 Conan 包。
    仓库不再内置 third_party：全部 C/C++ 第三方依赖都在这里声明。csv2 使用
    `third_party/recipes/csv2` 的本地配方（Conan Center 未收录），首次构建前先
    `conan create third_party/recipes/csv2`。

    用法:
        conan install . -of build-msvc -pr:h conan/profiles/windows-msvc-static -pr:b default --build=missing
        cmake -S . -B build-msvc -DCMAKE_TOOLCHAIN_FILE=build-msvc/conan_toolchain.cmake ...
    """

    required_conan_version = ">=2.0"

    settings = "os", "arch", "compiler", "build_type"

    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("openssl/3.6.4")
        self.requires("libiconv/1.19")
        self.requires("zlib/1.3.2")
        # Conan Center 只提供 minizip 1.2.x，现有代码仅使用 unz* API
        self.requires("minizip/1.2.12")
        self.requires("yaml-cpp/0.9.0")
        self.requires("mimalloc/3.5.1")
        self.requires("cpr/1.14.2")
        self.requires("capnproto/1.5.0")
        self.requires("benchmark/1.9.5")
        self.requires("catch2/3.16.0")
        self.requires("gtest/1.18.0")
        # 原 third_party 内置组件，全部改为 Conan 依赖
        self.requires("fmt/11.2.0")
        self.requires("spdlog/1.15.3")
        self.requires("asio/1.38.2")
        self.requires("pfr/2.2.0")  # 反射（原 boost.pfr 的独立版）
        self.requires("backward-cpp/1.6")
        self.requires("bshoshany-thread-pool/5.1.0")
        self.requires("croncpp/2023.03.30")
        self.requires("date/3.0.5")
        self.requires("cpp-httplib/0.65.0")
        self.requires("csv2/0.2")  # 本地配方：conan create third_party/recipes/csv2
        self.requires("magic_enum/0.9.8")
        self.requires("mio/cci.20230303")
        self.requires("nlohmann_json/3.11.3")
        self.requires("tsl-robin-map/1.4.0")
        self.requires("protobuf/3.21.12")
        # 数值容器与 SIMD 后端：xtensor 传递引入 xtl / xsimd
        self.requires("xtensor/0.27.2")

    def configure(self):
        # 全静态构建，与项目的 BUILD_SHARED_LIBS=OFF 保持一致
        self.options["*"].shared = False
        # spdlog 走 header-only：与项目原先的用法一致，也避免与外部 fmt 的 ABI 差异
        self.options["spdlog"].header_only = True
        # minizip 默认启用 bzip2，项目未使用，关闭以减少依赖
        self.options["minizip"].bzip2 = False
        # 项目不使用 FIPS 模块，关闭后显著缩短 OpenSSL 源码构建时间
        self.options["openssl"].no_fips = True
