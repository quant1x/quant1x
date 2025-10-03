#include <iostream>
#include <string>
#include <map>
#include <yaml-cpp/yaml.h>
#include "../quant1x/encoding/yaml.h"

struct CrontabItem { bool enable=false; std::string trigger; };

int main(int argc, char** argv) {
    const std::string path = (argc>1)?argv[1]:"D:/projects/quant1x/quant1x/examples/quant1x.yaml";
    YAML::Node root = YAML::LoadFile(path);
    auto node = root["runtime"]["crontab"];
    std::map<std::string, CrontabItem> m;
    try {
        encoding::yaml::deserialize_map(node, m);
        std::cout << "deserialize_map succeeded, entries=" << m.size() << "\n";
        for (auto &p: m) std::cout << "  key="<<p.first<<" enable="<<p.second.enable<<" trigger='"<<p.second.trigger<<"'\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "EX: "<<e.what()<<"\n"; return 2;
    } catch (...) {
        std::cerr << "EX: unknown\n"; return 3;
    }
}
