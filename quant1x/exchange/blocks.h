#pragma once
#ifndef QUANT1X_EXCHANGE_BLOCKS_H
#define QUANT1X_EXCHANGE_BLOCKS_H 1

//============================================================
// exchange 板块信息相关                                       //
//============================================================

#include <quant1x/std/api.h>

// 数据集
namespace exchange {

    // 板块信息
    struct block_info{
        std::string code;  // 板块代码
        std::string name;  // 板块名称
        u16 type = 0;      // 板块类型
        u16 num = 0;       // 成分股数量
        std::string Block; // 通达信板块编码
        std::vector<std::string> ConstituentStocks; // 成分股

        friend std::ostream &operator<<(std::ostream &os, const block_info &info);
    };
    // 下载板块原始数据
    void download_block_raw_data(const std::string &filename);
    // 解析板块原始数据
    std::vector<block_info> parse_block_raw_data(const std::string &filename);
    // 同步板块数据
    std::vector<block_info> sync_block_files();
    // 获取板块列表
    std::vector<block_info> get_sector_list();

    // 获取板块信息
    std::optional<block_info> get_sector_info(const std::string &code);

} // namespace exchange

#endif //QUANT1X_EXCHANGE_BLOCKS_H
