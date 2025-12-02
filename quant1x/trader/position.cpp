#include <quant1x/trader/position.h>
#include <quant1x/exchange/exchange.h>
#include <quant1x/encoding/csv.h>

namespace trader {

    static inline std::unordered_map<std::string, Position> mapPositions;
    static inline std::mutex                                positionsMutex;
    static inline auto positionOnce = RollingOnce::create("trader-position", exchange::cron_expr_daily_9am);

    const std::string qmtPositionsPath = "qmt";           // 持仓缓存路径
    const std::string qmtPositionsFilename = "positions.csv"; // 持仓数据文件名

    // 获取持仓缓存路径
    std::string getPositionsPath() {
        std::string rootPath = config::default_cache_path(); // 假设已实现
        return rootPath + "/" + qmtPositionsPath;
    }

    // 获取持仓缓存文件名
    std::string positionsFilename() {
        auto const & traderParameter = config::TraderConfig();
        std::string path = getPositionsPath();
        return path + "/" + traderParameter->AccountId + "-" + qmtPositionsFilename;
    }

    // 用证券代码作为关键字
    std::string Position::Key() const {
        return this->SecurityCode;
    }

    // 同步持仓数据
    bool Position::Sync(const PositionDetail& other) {
        this->AccountType = other.AccountType;
        this->AccountId = other.AccountId;
        this->SecurityCode = exchange::CorrectSecurityCode(other.StockCode);
        this->Volume = other.Volume;
        this->CanUseVolume = other.CanUseVolume;
        this->OpenPrice = other.OpenPrice;
        this->MarketValue = other.MarketValue;
        this->FrozenVolume = other.FrozenVolume;
        this->OnRoadVolume = other.OnRoadVolume;
        this->YesterdayVolume = other.YesterdayVolume;
        this->AvgPrice = other.AvgPrice;

        if (this->CreateTime.empty() && other.YesterdayVolume > 0) {
            exchange::timestamp ts = exchange::prev_trading_day();
            std::string frontDate = ts.only_date() + " 00:00:00";
            this->CreateTime = frontDate;
        }

        return true;
    }

    // MergeFromOrder 订单合并到持仓
    bool Position::MergeFromOrder(const OrderDetail& order, double price) {
        if (order.TradedVolume == 0) {
            return false;
        }

        bool plus = (order.OrderType == STOCK_BUY);

        double openValue = OpenPrice * Volume;
        double orderValue = order.TradedPrice * order.TradedVolume;

        if (plus) {
            Volume += order.TradedVolume;
            OnRoadVolume += order.TradedVolume;
            OpenPrice = (openValue + orderValue) / Volume;
            BuyTime = order.OrderTime;
            BuyPrice = order.TradedPrice;
            BuyVolume = order.TradedVolume;
        } else {
            Volume -= order.TradedVolume;
            if (Volume < 0) Volume = 0;

            CanUseVolume -= order.TradedVolume;
            if (CanUseVolume < 0) CanUseVolume = 0;

            SellTime = order.OrderTime;
            SellPrice = order.TradedPrice;
            SellVolume = order.TradedVolume;

            if (Volume > 0) {
                OpenPrice = (openValue - orderValue) / Volume;
            }
        }

        MarketValue = price * Volume;
        UpdateTime = order.OrderTime;

        return true;
    }

    // 加载本地的持仓数据
    void lazyLoadLocalPositions() {
        auto filename = positionsFilename();
        auto list = encoding::csv::csv_to_slices<Position>(filename);
        for(auto const & v: list) {
            mapPositions.emplace(v.SecurityCode, v);
        }
    }


    // SyncPositions 同步持仓
    void SyncPositions() {
        positionOnce->Do(lazyLoadLocalPositions);

        std::vector<PositionDetail> list = QueryHolding();
        if (list.empty()) return;

        std::lock_guard<std::mutex> lock(positionsMutex);

        for (const auto& v : list) {
            std::string code = exchange::CorrectSecurityCode(v.StockCode);
            auto [it, inserted] = mapPositions.try_emplace(code);
            Position& position = it->second;
            position.Sync(v);
        }
    }

    // UpdatePositions 更新持仓
    void UpdatePositions() {
        positionOnce->Do(lazyLoadLocalPositions);

        std::vector<OrderDetail> list = QueryOrders();
        if (list.empty()) return;

        std::lock_guard<std::mutex> lock(positionsMutex);

        for (const auto& v : list) {
            std::string code = exchange::CorrectSecurityCode(v.StockCode);
            auto [it, inserted] = mapPositions.try_emplace(code);
            Position& position = it->second;

            double price = 0.00;
            try {
                auto [mid, mflag, symbol] = exchange::DetectMarket(code);
                tsl::robin_map<std::string, level1::StockInfo> maps;
                maps[code] = level1::StockInfo{mid, symbol};
                std::vector<std::string> codes={code};
                level1::SecurityQuoteRequest request(codes);
                level1::SecurityQuoteResponse response;
                auto conn = level1::get_std_conn();
                auto err = level1::process(conn->socket(), request, response);
                if (err) {
                    spdlog::error("Process error: {}", err.message());
                    err.clear();
                    continue;
                }
                response.verify_delisted_securities(maps);
            } catch (...) {
                //
            }

            if(price > 0.00) {
                bool ok = position.MergeFromOrder(v, price);
                if (ok) {
                    // 可选更新操作
                }
            }
        }
    }

    // CacheSync 缓存同步到文件
    void CacheSync() {
        const std::string methodName = "CacheSync";
        positionOnce->Do(lazyLoadLocalPositions);

        std::lock_guard<std::mutex> lock(positionsMutex);

        size_t length = mapPositions.size();
        std::vector<Position> list;
        list.reserve(length);

        for (const auto& [key, value] : mapPositions) {
            list.push_back(value);
        }

        std::string cacheFilename = positionsFilename();
        encoding::csv::slices_to_csv(list, cacheFilename);
    }

} // namespace trader