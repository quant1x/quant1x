#pragma once
#ifndef QUANT1X_FACTOR_F10_SHARE_HOLDER_H
#define QUANT1X_FACTOR_F10_SHARE_HOLDER_H 1

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace dfcf {

    const int errorCapitalBase = 90000; // 股本异常错误码基础值
    const std::string urlEastmoneyGdfxHoldingAnalyse = "https://datacenter-web.eastmoney.com/api/data/v1/get";
    const int EastmoneyGdfxHoldingAnalysePageSize = 500;

    typedef int HoldNumChangeState;

    const HoldNumChangeState HoldNumDampened = -1;       // 减少
    const HoldNumChangeState HoldNumUnChanged = 0;       // 不变
    const HoldNumChangeState HoldNumNewlyAdded = 1;      // 新进/新增
    const HoldNumChangeState HoldNumIncrease = 2;        // 增加
    const HoldNumChangeState HoldNumUnknownChanges = -9; // 未知变化

    // CirculatingShareholder struct
    struct CirculatingShareholder {
        std::string SecurityCode;       // 证券代码
        std::string SecurityName;       // 证券名称
        std::string EndDate;            // 报告日期
        std::string UpdateDate;         // 更新日期
        std::string HolderType;         // 股东类型
        std::string HolderName;         // 股东名称
        std::string IsHoldOrg;          // 股东是否机构
        int HolderRank;                 // 股东排名
        int HoldNum;                    // 期末持股-数量
        double FreeHoldNumRatio;       // 期末持股-比例
        int HoldNumChange;             // 期末持股-持股变动
        std::string HoldChangeName;    // 期末持股-变化状态
        int HoldChangeState;           // 期末持股-变化状态
        double HoldChangeRatio;        // 期末持股-持股变化比例
        double HoldRatio;              // 期末持股-持股变动
        double HoldRatioChange;        // 期末持股-数量变化比例
    };

    // GetCacheShareHolder 获取流动股东数据
    std::vector<CirculatingShareholder> GetCacheShareHolder(const std::string& securityCode, const std::string& date, int diff = 1);

} // namespace dfcf



#endif //QUANT1X_FACTOR_F10_SHARE_HOLDER_H
