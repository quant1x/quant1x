import os
from typing import TYPE_CHECKING, Any

from conan.tools.cmake import CMake, cmake_layout

from conan import ConanFile


class Csv2TestPackage(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"

    if TYPE_CHECKING:
        # conan 在加载配方时注入这些实例属性，基类把它们声明为 None，静态检查需要收窄。
        requires: Any
        cpp: Any

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires(self.tested_reference_str)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        self.run(os.path.join(self.cpp.build.bindirs[0], "csv2_quote_test"))
