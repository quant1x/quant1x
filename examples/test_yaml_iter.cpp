#include <iostream>
#include <string>
#include <yaml-cpp/yaml.h>

int main(int argc, char** argv) {
    const std::string path = (argc>1)?argv[1]:"D:/projects/quant1x/quant1x/examples/quant1x.yaml";
    try {
        YAML::Node root = YAML::LoadFile(path);
        auto cr = root["runtime"]["crontab"];
        std::cout << "crontab: defined=" << cr.IsDefined() << " isMap=" << cr.IsMap() << " size=" << cr.size() << "\n";
        for (const auto &p : cr) {
            try {
                std::string k = p.first.as<std::string>();
                std::cout << "key:'" << k << "'\n";
            } catch(...) { std::cout << "key: <error>\n"; }
        }
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "EX: " << e.what() << "\n";
        return 2;
    }
}
