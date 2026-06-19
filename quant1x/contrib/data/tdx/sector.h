#pragma once
#ifndef QUANT1X_CONTRIB_DATA_TDX_SECTOR_H
#define QUANT1X_CONTRIB_DATA_TDX_SECTOR_H 1

/// sector — 板块列表加载与下载, 对齐 Python contrib/data/tdx/sector.py

#include <quant1x/data/schema/sector.h>
#include <string>
#include <vector>
#include <optional>

namespace quant1x::contrib::data::tdx::sector {

    /// 板块类型枚举 (对齐 Python SectorType)
    enum SectorType : int {
        UNKNOWN  = 0,   ///< 未知类型
        HANGYE   = 2,   ///< 行业
        DIQU     = 3,   ///< 地区
        GAINIAN  = 4,   ///< 概念
        FENGGE   = 5,   ///< 风格
        ZHISHU   = 6,   ///< 指数
        YJHY     = 12,  ///< 研究行业
    };

    /// 通过板块类型代码获取板块类型名称
    std::string sector_type_name_by_code(int sector_code);

    /// 获取板块缓存文件路径 (对齐 Python get_sector_filename)
    std::string get_sector_filename();

    /// 获取全部板块列表 (对齐 Python get_sector_list)
    /// 首次调用时会自动触发板块文件同步 (下载 + 解析 + 生成CSV缓存)
    std::vector<quant1x::data::schema::Sector> get_sector_list();

    /// 根据代码获取板块信息 (对齐 Python get_sector_info)
    std::optional<quant1x::data::schema::Sector> get_sector_info(const std::string &symbol);

} // namespace quant1x::contrib::data::tdx::sector

#endif // QUANT1X_CONTRIB_DATA_TDX_SECTOR_H
