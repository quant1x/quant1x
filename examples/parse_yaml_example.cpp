#include <iostream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

// 相对包含路径：从 examples 目录到 header 的相对路径
#include "../quant1x/encoding/yaml.h"
#include "config.h"

using namespace std;

// types are declared in examples/config.h

int main(int argc, char **argv) {
    const std::string path = (argc > 1) ? argv[1] : "examples/quant1x.yaml";

    try {
        // debug: print path and whether file exists
        std::ifstream fcheck(path);
        if (!fcheck.good()) {
            spdlog::warn("YAML file not found or inaccessible: '{}'", path);
        } else {
            spdlog::info("YAML file exists: '{}'", path);
        }
        YAML::Node root = YAML::LoadFile(path);
        spdlog::debug("root.IsNull={} IsMap={} size={}", root.IsNull()?"true":"false", root.IsMap()?"true":"false", root.size());
        if (root["trader"]) spdlog::debug("root[trader] present"); else spdlog::debug("root[trader] missing");
        if (root["trader"] && root["trader"]["strategies"]) spdlog::debug("strategies size={}", root["trader"]["strategies"].size());

        // 使用 encoding::yaml 的 pfr 反射反序列化已定义的结构体子集
        Config cfg;
            try {
                cfg = encoding::yaml::deserialize<Config>(root);
                spdlog::info("deserialize completed, trader.strategies.size={}", cfg.trader.strategies.size());
            } catch (const std::exception &e) {
                spdlog::error("Deserialize std::exception: {}", e.what());
                return 2;
            } catch (...) {
                spdlog::error("Deserialize unknown exception (non-std)");
                return 3;
            }

        // 由于编码器现在会将成员名映射到更合理的 YAML 键（例如 is_auto -> "auto"），
        // 手工补字段已不再需要。下面只处理 runtime.data 等动态键的 map 类型。

        // runtime.crontab: 将子节点转换为 map<string, CrontabItem>
        if (root["runtime"] && root["runtime"]["crontab"]) {
            try {
                auto tmp = root["runtime"]["crontab"].as<std::map<std::string, YAML::Node>>();
                for (auto &kv : tmp) {
                    const std::string key = kv.first;
                    CrontabItem item;
                    YAML::Node &val = kv.second;
                    if (val["enable"]) try { item.enable = val["enable"].as<bool>(); } catch(...) {}
                    if (val["trigger"]) try { item.trigger = val["trigger"].as<std::string>(); } catch(...) {}
                    cfg.runtime.crontab.emplace(key, std::move(item));
                }
            } catch(...) {
                // skip on parse error
            }
        }

        // data.concurrency: map
        if (root["data"] && root["data"]["concurrency"]) {
            try {
                auto tmp = root["data"]["concurrency"].as<std::map<std::string, YAML::Node>>();
                for (auto &kv: tmp) {
                    try { cfg.data.concurrency[kv.first] = kv.second.as<int>(); } catch(...) {}
                }
            } catch(...) {}
        }

        // data.cache.kline: 将任意子键映射为 bool
        if (root["data"] && root["data"]["cache"] && root["data"]["cache"]["kline"]) {
            try {
                auto tmp = root["data"]["cache"]["kline"].as<std::map<std::string, YAML::Node>>();
                for (auto &kv: tmp) {
                    try { cfg.data.cache.kline[kv.first] = kv.second.as<bool>(); } catch(...) {}
                }
            } catch(...) {}
        }

        // data.cache.chips.index_enabled
        if (root["data"] && root["data"]["cache"] && root["data"]["cache"]["chips"]) {
            try { cfg.data.cache.chips.index_enabled = root["data"]["cache"]["chips"]["index_enabled"].as<bool>(); } catch(...) {}
        }

        // data.trans.begin_date
        if (root["data"] && root["data"]["trans"]) {
            try { cfg.data.trans.begin_date = root["data"]["trans"]["begin_date"].as<std::string>(); } catch(...) {}
        }

        // feature.wave.fields
        if (root["data"] && root["data"]["feature"] && root["data"]["feature"]["wave"]) {
            auto wf = root["data"]["feature"]["wave"];
            try { cfg.data.feature.wave.periods = wf["periods"].as<int>(); } catch(...) {}
            if (wf["fields"]) {
                try { cfg.data.feature.wave.fields.peak = wf["fields"]["peak"].as<std::string>(); } catch(...) {}
                try { cfg.data.feature.wave.fields.valley = wf["fields"]["valley"].as<std::string>(); } catch(...) {}
            }
        }

        // snapshot
        if (root["data"] && root["data"]["snapshot"]) {
            try { cfg.data.snapshot.concurrency = root["data"]["snapshot"]["concurrency"].as<int>(); } catch(...) {}
        }

        // 打印详细摘要
        cout << "Configuration Summary:\n";
        cout << "  basedir: " << cfg.basedir << "\n";
        cout << "  debug: " << (cfg.debug?"true":"false") << "\n";
        cout << "Trader:\n";
        cout << "  account_id: " << cfg.trader.account_id << "\n";
        cout << "  proxy_url: " << cfg.trader.proxy_url << "\n";
        cout << "  order_path: " << cfg.trader.order_path << "\n";
        cout << "  stamp duty buy/sell: " << cfg.trader.stamp_duty_rate_for_buy << "/" << cfg.trader.stamp_duty_rate_for_sell << "\n";
        cout << "  commission_rate/min: " << cfg.trader.commission_rate << "/" << cfg.trader.commission_min << "\n";
        cout << "  position_ratio: " << cfg.trader.position_ratio << " keep_cash: " << cfg.trader.keep_cash << "\n";
        cout << "  strategies: " << cfg.trader.strategies.size() << "\n";
        for (size_t i = 0; i < cfg.trader.strategies.size(); ++i) {
            const auto &s = cfg.trader.strategies[i];
            cout << "   - id=" << s.id << " name='" << s.name << "' auto=" << (s.auto_?"true":"false") << " flag=" << s.flag << " time=" << s.time << " total=" << s.total << "\n";
        }

        cout << "Runtime crontab entries: " << cfg.runtime.crontab.size() << "\n";
        for (const auto &p: cfg.runtime.crontab) {
            cout << "  " << p.first << ": enable=" << (p.second.enable?"true":"false") << " trigger='" << p.second.trigger << "'\n";
        }

        cout << "Data.concurrency entries: " << cfg.data.concurrency.size() << "\n";
        for (const auto &p: cfg.data.concurrency) cout << "  " << p.first << " = " << p.second << "\n";

        cout << "Data.cache.kline:\n";
        for (const auto &p: cfg.data.cache.kline) cout << "  " << p.first << " = " << (p.second?"true":"false") << "\n";

        cout << "Data.cache.chips.index_enabled: " << (cfg.data.cache.chips.index_enabled?"true":"false") << "\n";
        cout << "Data.trans.begin_date: " << cfg.data.trans.begin_date << "\n";
        cout << "Data.feature.wave.periods: " << cfg.data.feature.wave.periods << "\n";
        cout << "Data.feature.wave.fields.peak/valley: " << cfg.data.feature.wave.fields.peak << "/" << cfg.data.feature.wave.fields.valley << "\n";
        cout << "Data.snapshot.concurrency: " << cfg.data.snapshot.concurrency << "\n";

        return 0;
    } catch (const std::exception &ex) {
        spdlog::critical("Error loading YAML: {}", ex.what());
        return 2;
    }
}
