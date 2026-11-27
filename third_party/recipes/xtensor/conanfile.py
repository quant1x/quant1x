import os
from typing import TYPE_CHECKING, Any, ClassVar, cast

from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain
from conan.tools.files import copy, get, rmdir
from conan.tools.layout import basic_layout
from conan.tools.scm import Version

from conan import ConanFile

required_conan_version = ">=2"


class XtensorConan(ConanFile):
    """xtensor 本地配方。

    Conan Center 目前只到 0.27.1，这里按 CCI 配方结构补 0.27.2（源码取自
    xtensor-stack/xtensor 的 0.27.2 tag）。首次构建前执行：

        conan create third_party/recipes/xtensor

    依赖版本与 CCI 一致：xtl/0.8.0、nlohmann_json/3.11.3、xsimd/13.2.0。
    """

    name = "xtensor"
    version = "0.27.2"
    package_type = "header-library"
    description = "C++ tensors with broadcasting and lazy computing"
    license = "BSD-3-Clause"
    url = "https://codeberg.org/quant1x/quant1x"
    homepage = "https://github.com/xtensor-stack/xtensor"
    topics = ("numpy", "multidimensional-arrays", "tensors")
    settings = "os", "arch", "compiler", "build_type"
    options: ClassVar[Any] = {
        "xsimd": [True, False],
        "tbb": [True, False],
        "openmp": [True, False],
    }
    default_options: ClassVar[Any] = {
        "xsimd": True,
        "tbb": False,
        "openmp": False,
    }

    if TYPE_CHECKING:
        # conan 在加载配方时注入这些实例属性，基类把它们声明为 None，静态检查需要收窄。
        requires: Any
        tool_requires: Any
        info: Any
        conan_data: Any

    def layout(self):
        basic_layout(self, src_folder="src")

    def requirements(self):
        # https://github.com/xtensor-stack/xtensor?tab=readme-ov-file#dependencies
        if Version(self.version) < "0.26.0":
            self.requires("xtl/0.7.5")
        else:
            self.requires("xtl/0.8.0")
        self.requires("nlohmann_json/3.11.3")
        if self.options.xsimd:
            if Version(self.version) < "0.26.0":
                self.requires("xsimd/13.0.0")
            else:
                self.requires("xsimd/13.2.0")
        if self.options.tbb:
            self.requires("onetbb/[>=2021.10.0 <2024]")

    def build_requirements(self):
        # required by newer versions of xtensor
        self.tool_requires("cmake/[>=3.29]")

    def package_id(self):
        self.info.clear()

    def validate(self):
        if self.options.tbb and self.options.openmp:
            raise ConanInvalidConfiguration(
                "The options 'tbb' and 'openmp' can not be used together."
            )

        if Version(self.version) >= 0.27:
            check_min_cppstd(self, 20)
        else:
            check_min_cppstd(self, 14 if Version(self.version) < "0.26.0" else 17)

    def source(self):
        get(self, **self.conan_data["sources"][self.version],
            destination=cast(str, self.source_folder), strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)
        if Version(self.version) < "0.26.0":
            tc.cache_variables["CMAKE_POLICY_VERSION_MINIMUM"] = "3.5"  # CMake 4 support
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        package_folder = cast(str, self.package_folder)
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(package_folder, "share"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "xtensor")
        self.cpp_info.set_property("cmake_target_name", "xtensor")
        self.cpp_info.set_property("pkg_config_name", "xtensor")
        self.cpp_info.requires = ['xtl::xtl', 'nlohmann_json::nlohmann_json']
        if self.options.xsimd:
            self.cpp_info.requires.append('xsimd::xsimd')
        if self.options.tbb:
            self.cpp_info.requires.append('onetbb::libtbb')
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        if self.options.xsimd:
            self.cpp_info.defines.append("XTENSOR_USE_XSIMD")
        if self.options.tbb:
            self.cpp_info.defines.append("XTENSOR_USE_TBB")
        if self.options.openmp:
            self.cpp_info.defines.append("XTENSOR_USE_OPENMP")
