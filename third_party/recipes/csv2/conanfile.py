from conan import ConanFile
from conan.errors import ConanException
from conan.tools.files import copy, get, load, replace_in_file
import os

# csv2 v0.2 的 read_value 只折叠字段内部的 ""，不去掉字段最外层的一对引号，
# 导致 `"a,b"`、`"a""b"` 这类值原样带回引号。这里在打包时定点修复：
# 先剥掉最外层引号，再折叠内部的 ""。
_READ_VALUE_OLD = '''    // If cell is escaped, convert and return correct cell contents,
    // e.g., """foo""" => ""foo""
    template <typename Container> void read_value(Container &result) const {
      if (start_ >= end_)
        return;
      const auto new_start_end = trim_policy::trim(buffer_, start_, end_);
      result.insert(result.end(), buffer_ + new_start_end.first, buffer_ + new_start_end.second);
      // An empty quoted field ("") has no content to escape and must resolve
      // to an empty value rather than a single leftover quote character.
      if (result.size() == 2 && result[0] == quote_character::value &&
          result[1] == quote_character::value) {
        result.clear();
        return;
      }
      for (size_t i = 1; i < result.size(); ++i) {
        if (result[i] == quote_character::value && result[i - 1] == quote_character::value) {
          result.erase(i - 1, 1);
        }
      }
    }'''

_READ_VALUE_NEW = '''    // If cell is escaped, convert and return correct cell contents,
    // e.g., """foo""" => "foo"
    template <typename Container> void read_value(Container &result) const {
      if (start_ >= end_)
        return;
      const auto new_start_end = trim_policy::trim(buffer_, start_, end_);
      size_t begin = new_start_end.first;
      size_t finish = new_start_end.second;
      // 去掉字段最外层的一对引号，再处理内部的 ""。
      if (finish - begin >= 2 && buffer_[begin] == quote_character::value &&
          buffer_[finish - 1] == quote_character::value) {
        ++begin;
        --finish;
      }
      result.insert(result.end(), buffer_ + begin, buffer_ + finish);
      for (size_t i = 1; i < result.size(); ++i) {
        if (result[i] == quote_character::value && result[i - 1] == quote_character::value) {
          result.erase(i - 1, 1);
        }
      }
    }'''


class Csv2Conan(ConanFile):
    """csv2（p-ranav/csv2）—— Conan Center 未收录，配方随仓库维护配方随仓库维护。

    v0.2v0.2 的引号解析有缺陷（外层引号不剥离），在的引号解析有缺陷（外层引号不剥离），在 build()build 里定点修复阶段把源码复制到build
    目录后定点修复，源缓存保持原样。
    """

    name = "csv2"
    version = "0.2"
    url = "https://github.com/p-ranav/csv2"
    homepage = "https://github.com/p-ranav/csv2"
    description = "Fast CSV reader/writer for C++"
    license = "MIT"
    package_type = "header-library"
    settings = "os", "arch", "compiler", "build_type"
    no_copy_source = True

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def _patched_include(self) -> str:
        return os.path.join(self.build_folder, "include")

    def build(self):
        # 不修改 source 缓存：复制到 build 目录后再打补丁
        copy(self, "*.hpp", src=os.path.join(self.source_folder, "include"),
             dst=self._patched_include())
        copy(self, "LICENSE*", src=self.source_folder, dst=self.build_folder)

        reader = os.path.join(self._patched_include(), "csv2", "reader.hpp")
        content = load(self, reader)
        if _READ_VALUE_NEW in content:
            return
        if _READ_VALUE_OLD not in content:
            raise ConanException(
                "csv2 reader.hpp 的 read_value 结构已变化，请重新核对第三方的引号补丁")
        replace_in_file(self, reader, _READ_VALUE_OLD, _READ_VALUE_NEW)

    def package(self):
        copy(self, "*.hpp", src=self._patched_include(),
             dst=os.path.join(self.package_folder, "include"))
        copy(self, "LICENSE*", src=self.build_folder,
             dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "csv2")
        self.cpp_info.set_property("cmake_target_name", "csv2::csv2")
        self.cpp_info.includedirs = ["include"]
