#include <quant1x/config/strategy_parameter.h>
#include <quant1x/data/market/instruments.h>
#include <quant1x/data/exchange/margin_trading.h>

namespace config {

    // 初始化排除列表
    void StrategyParameter::initExclude() {
        if (!excludeCodes.empty()) {
            return;
        }

        std::vector<std::string> tempExcludeCodes;
        for (const auto& v : Sectors) {
            std::string sectorCode = strings::trim(v);
            if (sectorCode.starts_with(sectorIgnorePrefix)) {
                sectorCode = strings::trim(sectorCode.substr(sectorPrefixLength));
                auto blockInfo = exchange::get_sector_info(sectorCode);
                if (blockInfo.has_value()) {
                    tempExcludeCodes.insert(tempExcludeCodes.end(),
                                            blockInfo->ConstituentStocks.begin(),
                                            blockInfo->ConstituentStocks.end());
                }
            }
        }

        // 去重
        std::unordered_set<std::string> uniqueCodes(tempExcludeCodes.begin(), tempExcludeCodes.end());
        excludeCodes.assign(uniqueCodes.begin(), uniqueCodes.end());
    }

    // 过滤股票代码
    std::vector<std::string> StrategyParameter::Filter(const std::vector<std::string>& codes) {
        initExclude();

        // 过滤需要忽略的板块成份股
        std::vector<std::string> newCodeList;
        std::copy_if(codes.begin(), codes.end(), std::back_inserter(newCodeList),
                     [this](const std::string& s) {
                         return std::find(excludeCodes.begin(), excludeCodes.end(), s) == excludeCodes.end();
                     });

        // 去重
        std::unordered_set<std::string> uniqueCodes(newCodeList.begin(), newCodeList.end());
        newCodeList.assign(uniqueCodes.begin(), uniqueCodes.end());

        // 排序
        std::sort(newCodeList.begin(), newCodeList.end());

        if (IgnoreMarginTrading) {
            // 过滤两融
            auto marginTradingList = exchange::MarginTradingList();
            std::unordered_set<std::string> marginSet(marginTradingList.begin(), marginTradingList.end());

            std::vector<std::string> filteredList;
            std::copy_if(newCodeList.begin(), newCodeList.end(), std::back_inserter(filteredList),
                         [&marginSet](const std::string& s) {
                             return marginSet.find(s) == marginSet.end();
                         });
            return filteredList;
        }
        return newCodeList;
    }

    // 取得可以交易的证券代码列表
    std::vector<std::string> StrategyParameter::StockList() {
        std::vector<std::string> codes;
        for (const auto& v : Sectors) {
            std::string sectorCode = strings::trim(v);
            if (!sectorCode.starts_with(sectorIgnorePrefix)) {
                auto blockInfo = exchange::get_sector_info(sectorCode);
                if (blockInfo.has_value()) {
                    codes.insert(codes.end(),
                                 blockInfo->ConstituentStocks.begin(),
                                 blockInfo->ConstituentStocks.end());
                }
            }
        }

        if (codes.empty()) {
            codes = instruments::GetStockCodeList();
        }
        return Filter(codes);
    }

    std::ostream &operator<<(std::ostream &os, const StrategyParameter &parameter) {
        os << "{Id: " << parameter.Id  << "\n"
           << " Auto: " << parameter.Auto  << "\n"
           << " Name: " << parameter.Name << "\n"
           << " Flag: " << parameter.Flag << "\n"
           << " Session: " << parameter.Session.ToString() << "\n"
           << " Weight: " << parameter.Weight << "\n"
           << " Total: " << parameter.Total << "\n"
           << " PriceCageRatio: " << parameter.PriceCageRatio << "\n"
           << " MinimumPriceFluctuationUnit: " << parameter.MinimumPriceFluctuationUnit << "\n"
           << " FeeMax: " << parameter.FeeMax << "\n"
           << " FeeMin: " << parameter.FeeMin << "\n"
           //<< " Sectors: " << parameter.Sectors << "\n"
           << " IgnoreMarginTrading: " << parameter.IgnoreMarginTrading << "\n"
           << " HoldingPeriod: " << parameter.HoldingPeriod << "\n"
           << " SellStrategy: " << parameter.SellStrategy << "\n"
           << " FixedYield: " << parameter.FixedYield << "\n"
           << " TakeProfitRatio: " << parameter.TakeProfitRatio << "\n"
           << " StopLossRatio: " << parameter.StopLossRatio << "\n"
           << " LowOpeningAmplitude: " << parameter.LowOpeningAmplitude << "\n"
           << " HighOpeningAmplitude: " << parameter.HighOpeningAmplitude << "\n"
           << " Rules: " << parameter.Rules
           //<< " excludeCodes: " << parameter.excludeCodes;
           << "}";
        return os;
    }
}