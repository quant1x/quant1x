#include <quant1x/test/test.h>
#include <quant1x/backtest/backtest.h>

#include <iostream>
#include <string>
#include <unordered_map>

// Position 结构体模拟 Go 中的持仓
struct Position {
    std::string SecurityCode;
    int Volume = 0;

    // 模拟 Sync 方法
    void Sync(const std::string& code, int volume) {
        SecurityCode = code;
        Volume += volume;
    }
};

// 打印函数，用于调试
void PrintMap(const std::unordered_map<std::string, Position>& map) {
    for (const auto& [key, pos] : map) {
        std::cout << "Key: " << key << ", "
                  << "SecurityCode: " << pos.SecurityCode << ", "
                  << "Volume: " << pos.Volume << std::endl;
    }
}

TEST_CASE("order-basic", "[backtest]") {
    std::unordered_map<std::string, Position> mapPositions;

    // 模拟 QueryHolding 返回的数据
    struct PositionDetail {
        std::string StockCode;
        int Volume;
    };

    std::vector<PositionDetail> list = {
        {"600000.SH", 100},
        {"600001.SH", 200},
        {"600000.SH", 50},   // 同一标的再次出现
        {"600002.SH", 300},
        {"600001.SH", 100}, // 更新已存在标的
    };

    // 模拟 SyncPositions 中的逻辑
    for (const auto& v : list) {
        std::string code = v.StockCode;
        auto [it, inserted] = mapPositions.try_emplace(code);

        Position& position = it->second;

        // 不管是新增还是已有项，都调用 Sync
        position.Sync(code, v.Volume);

        std::cout << (inserted ? "Inserted" : "Updated") << " - "
                  << "code: " << code << ", "
                  << "volume: " << position.Volume << std::endl;
    }

    // 输出最终结果
    std::cout << "\nFinal map state:\n";
    PrintMap(mapPositions);
}