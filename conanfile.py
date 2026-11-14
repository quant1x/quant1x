from conan import ConanFile


class Quant1XDependencies(ConanFile):
    """Quant1X C++ 第三方依赖（Conan 2 consumer 配方）.

    仅用于声明依赖并生成 conan_toolchain.cmake / CMakeDeps 配置，
    不产出 Conan 包。protobuf 仍由 third_party 内置的 3.21.11 提供，
    因此不在此处声明。

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

    def configure(self):
        # 全静态构建，与项目的 BUILD_SHARED_LIBS=OFF 保持一致
        self.options["*"].shared = False
        # minizip 默认启用 bzip2，项目未使用，关闭以减少依赖
        self.options["minizip"].bzip2 = False
        # 项目不使用 FIPS 模块，关闭后显著缩短 OpenSSL 源码构建时间
        self.options["openssl"].no_fips = True
