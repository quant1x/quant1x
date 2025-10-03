#include <iostream>
#include <fstream>
#include "../quant1x/encoding/yaml.h"
#include <yaml-cpp/yaml.h>

struct Strategy { int id=0; std::string name; };
struct Trader { std::string account_id; std::vector<Strategy> strategies; };

int main(int argc, char **argv){
    const std::string path = (argc>1)?argv[1]:"D:/projects/quant1x/quant1x/examples/quant1x.yaml";
    std::ifstream f(path);
    if(!f.good()){ std::cerr<<"cannot open "<<path<<"\n"; return 1; }
    auto node = YAML::LoadFile(path);
    try {
        Trader t = encoding::yaml::deserialize<Trader>(node["trader"]);
        std::cout<<"trader.account_id='"<<t.account_id<<"' strategies="<<t.strategies.size()<<"\n";
        return 0;
    } catch(const std::exception &e) {
        std::cerr<<"EX: "<<e.what()<<"\n"; return 2;
    }
}
