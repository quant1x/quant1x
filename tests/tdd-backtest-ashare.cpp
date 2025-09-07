#include <test/test.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <unordered_set>

// 日期类
class Date {
public:
    int year = 1970;
    int month = 1;
    int day = 1;

    // 默认构造函数
    Date() = default;

    Date(int y, int m, int d) : year(y), month(m), day(d) {}

    bool operator<(const Date& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day < other.day;
    }

    bool operator==(const Date& other) const {
        return year == other.year && month == other.month && day == other.day;
    }

    std::string toString() const {
        return std::to_string(year) + "-" +
               (month < 10 ? "0" : "") + std::to_string(month) + "-" +
               (day < 10 ? "0" : "") + std::to_string(day);
    }

    // 计算是否是交易日（简化版，实际应从市场日历获取）
    bool isTradingDay() const {
        // 简单模拟：周末不是交易日
        int q = day;
        int m = month;
        int y = year;
        if (m < 3) {
            m += 12;
            y--;
        }
        int h = (q + (13*(m+1))/5 + y + y/4 - y/100 + y/400) % 7;
        return h != 0 && h != 1; // 周六周日休市
    }
};

// 为 Date 类提供哈希函数
namespace std {
    template<>
    struct hash<Date> {
        size_t operator()(const Date& d) const {
            return ((d.year * 10000 + d.month * 100 + d.day) * 2654435761) % 2^32;
        }
    };
}

// 价格数据点（增加涨跌停价）
struct PriceData {
    Date date;
    double open;
    double high;
    double low;
    double close;
    double volume;
    double upper_limit; // 涨停价
    double lower_limit; // 跌停价
    double prev_close;  // 前收盘价
};

// 持仓信息（增加买入日期用于T+1检查）
struct Position {
    std::string symbol;
    double quantity = 0.00;
    double entryPrice = 0.00;
    Date entryDate;
    Date buyDate; // 实际买入日期（用于T+1判断）

    // 默认构造函数
    Position() : entryDate(1970, 1, 1), buyDate(1970, 1, 1) {}
    // 带参数的构造函数
    Position(const std::string& sym, double qty, double price, const Date& eDate, const Date& bDate)
        : symbol(sym), quantity(qty), entryPrice(price), entryDate(eDate), buyDate(bDate) {}
};

// 交易记录
struct TradeRecord {
    Date date;
    std::string symbol;
    std::string action; // "BUY" or "SELL"
    double quantity;
    double price;
    double commission;
    std::string status; // "FILLED" or "REJECTED"
    std::string reason; // 拒绝原因
};

// 每日结算记录
struct DailyRecord {
    Date date;
    double capital;
    double realizedPnL;
    double unrealizedPnL;
    double totalValue;
    double dailyReturn;
};

class AShareBacktestEngine {
private:
    double initialCapital;
    Date startDate;
    Date endDate;
    std::vector<Date> tradingDates;

    std::vector<TradeRecord> tradeHistory;
    std::vector<DailyRecord> dailyRecords;

    std::unordered_map<std::string, std::vector<PriceData>> priceData;
    std::unordered_map<Date, std::unordered_set<std::string>> suspendedStocks; // 停牌股票

    // 交易参数
    double commissionRate = 0.0003;  // 佣金率 0.03%
    double stampTax = 0.001;         // 印花税 0.1%（仅卖出收取）
    double minCommission = 5.0;      // 最低佣金5元

protected:
    std::unordered_map<std::string, Position> positions;
    double currentCapital;
public:
    AShareBacktestEngine(double capital, const Date& start, const Date& end)
        : initialCapital(capital), startDate(start), endDate(end), currentCapital(capital) {
        generateTradingDates();
    }

    // 生成交易日期范围
    void generateTradingDates() {
        Date current = startDate;
        while (!(current == endDate)) {
            if (current.isTradingDay()) {
                tradingDates.push_back(current);
            }
            incrementDate(current);
        }
        if (endDate.isTradingDay()) {
            tradingDates.push_back(endDate);
        }
    }

    virtual // 加载价格数据
    void loadPriceData(const std::string& symbol, const std::vector<PriceData>& data) {
        priceData[symbol] = data;

        // 记录停牌日
        for (const auto& day : data) {
            if (day.volume <= 0 || day.open <= 0) {
                suspendedStocks[day.date].insert(symbol);
            }
        }
    }

    // 计算交易费用
    double calculateCommission(double amount, bool isBuy) {
        double commission = amount * commissionRate;
        commission = std::max(commission, minCommission);
        if (!isBuy) {
            commission += amount * stampTax; // 卖出时加印花税
        }
        return commission;
    }

    // 买入（A股T+1）
    void buy(const std::string& symbol, double quantity, const Date& date) {
        if (quantity <= 0) {
            recordRejectedTrade(date, symbol, "BUY", quantity, 0, "Invalid quantity");
            return;
        }

        // 检查是否停牌
        if (isSuspended(symbol, date)) {
            recordRejectedTrade(date, symbol, "BUY", quantity, 0, "Stock suspended");
            return;
        }

        // 获取当前价格
        double price = getCurrentPrice(symbol, date);
        if (price <= 0) {
            recordRejectedTrade(date, symbol, "BUY", quantity, 0, "Invalid price");
            return;
        }

        // 检查是否涨停（A股涨停不能买入）
        if (price >= getUpperLimitPrice(symbol, date) - 1e-6) {
            recordRejectedTrade(date, symbol, "BUY", quantity, price, "Upper limit reached");
            return;
        }

        double amount = price * quantity;
        double commission = calculateCommission(amount, true);
        double totalCost = amount + commission;

        if (totalCost > currentCapital) {
            recordRejectedTrade(date, symbol, "BUY", quantity, price, "Insufficient capital");
            return;
        }

        // 更新资金
        currentCapital -= totalCost;

        // 记录交易
        TradeRecord record{date, symbol, "BUY", quantity, price, commission, "FILLED", ""};
        tradeHistory.push_back(record);

        // 更新持仓（A股T+1，实际可卖日期是下一个交易日）
        if (positions.find(symbol) != positions.end()) {
            Position& pos = positions[symbol];
            double totalQty = pos.quantity + quantity;
            double avgPrice = (pos.entryPrice * pos.quantity + price * quantity) / totalQty;

            pos.quantity = totalQty;
            pos.entryPrice = avgPrice;
            // 买入日期更新为最新买入的日期
            pos.buyDate = date;
        } else {
            // 新建持仓
            Position pos{symbol, quantity, price, date, date};
            positions[symbol] = pos;
        }
    }

    // 卖出（A股T+1）
    void sell(const std::string& symbol, double quantity, const Date& date) {
        if (quantity <= 0) {
            recordRejectedTrade(date, symbol, "SELL", quantity, 0, "Invalid quantity");
            return;
        }

        // 检查持仓
        if (positions.find(symbol) == positions.end() || positions[symbol].quantity < quantity) {
            recordRejectedTrade(date, symbol, "SELL", quantity, 0, "Insufficient position");
            return;
        }

        // 检查T+1规则
        Position& pos = positions[symbol];
        if (date < getNextTradingDay(pos.buyDate)) {
            recordRejectedTrade(date, symbol, "SELL", quantity, 0, "T+1 restriction");
            return;
        }

        // 检查是否停牌
        if (isSuspended(symbol, date)) {
            recordRejectedTrade(date, symbol, "SELL", quantity, 0, "Stock suspended");
            return;
        }

        // 获取当前价格
        double price = getCurrentPrice(symbol, date);
        if (price <= 0) {
            recordRejectedTrade(date, symbol, "SELL", quantity, 0, "Invalid price");
            return;
        }

        // 检查是否跌停（A股跌停不能卖出）
        if (price <= getLowerLimitPrice(symbol, date) + 1e-6) {
            recordRejectedTrade(date, symbol, "SELL", quantity, price, "Lower limit reached");
            return;
        }

        double amount = price * quantity;
        double commission = calculateCommission(amount, false);
        double proceeds = amount - commission;

        // 更新资金
        currentCapital += proceeds;

        // 记录交易
        TradeRecord record{date, symbol, "SELL", quantity, price, commission, "FILLED", ""};
        tradeHistory.push_back(record);

        // 更新持仓
        pos.quantity -= quantity;
        if (pos.quantity <= 0.0001) { // 考虑浮点精度
            positions.erase(symbol);
        }
    }

    // 记录被拒绝的交易
    void recordRejectedTrade(const Date& date, const std::string& symbol,
                             const std::string& action, double quantity,
                             double price, const std::string& reason) {
        TradeRecord record{date, symbol, action, quantity, price, 0, "REJECTED", reason};
        tradeHistory.push_back(record);
    }

    // 获取当前价格
    double getCurrentPrice(const std::string& symbol, const Date& date) {
        if (priceData.find(symbol) == priceData.end()) {
            return -1.0;
        }

        for (const auto& data : priceData[symbol]) {
            if (data.date == date) {
                return data.close; // 使用收盘价
            }
        }

        return -1.0;
    }

    // 获取涨停价
    double getUpperLimitPrice(const std::string& symbol, const Date& date) {
        if (priceData.find(symbol) == priceData.end()) {
            return -1.0;
        }

        for (const auto& data : priceData[symbol]) {
            if (data.date == date) {
                return data.upper_limit;
            }
        }

        return -1.0;
    }

    // 获取跌停价
    double getLowerLimitPrice(const std::string& symbol, const Date& date) {
        if (priceData.find(symbol) == priceData.end()) {
            return -1.0;
        }

        for (const auto& data : priceData[symbol]) {
            if (data.date == date) {
                return data.lower_limit;
            }
        }

        return -1.0;
    }

    // 检查股票是否停牌
    bool isSuspended(const std::string& symbol, const Date& date) {
        auto it = suspendedStocks.find(date);
        if (it != suspendedStocks.end()) {
            return it->second.find(symbol) != it->second.end();
        }
        return false;
    }

    // 获取下一个交易日
    Date getNextTradingDay(Date date) {
        do {
            incrementDate(date);
        } while (!date.isTradingDay());
        return date;
    }

    // 计算未实现盈亏
    double calculateUnrealizedPnL(const Date& date) {
        double unrealized = 0.0;

        for (auto& pair : positions) {
            const std::string& symbol = pair.first;
            Position& pos = pair.second;

            double currentPrice = getCurrentPrice(symbol, date);
            if (currentPrice > 0) {
                unrealized += (currentPrice - pos.entryPrice) * pos.quantity;
            }
        }

        return unrealized;
    }

    // 每日结算
    void dailySettlement(const Date& date) {
        double unrealizedPnL = calculateUnrealizedPnL(date);
        double realizedPnL = 0.0;

        // 计算当日已实现盈亏（从交易记录中）
        for (const auto& trade : tradeHistory) {
            if (trade.date == date && trade.status == "FILLED" && trade.action == "SELL") {
                // 查找该股票的持仓成本
                if (positions.find(trade.symbol) != positions.end()) {
                    const Position& pos = positions[trade.symbol];
                    double pnl = (trade.price - pos.entryPrice) * trade.quantity - trade.commission;
                    realizedPnL += pnl;
                }
            }
        }

        double totalValue = currentCapital;
        for (const auto& pair : positions) {
            double price = getCurrentPrice(pair.first, date);
            if (price > 0) {
                totalValue += price * pair.second.quantity;
            }
        }

        // 计算日收益率
        double dailyReturn = 0.0;
        if (!dailyRecords.empty()) {
            double prevTotal = dailyRecords.back().totalValue;
            if (prevTotal > 0) {
                dailyReturn = (totalValue - prevTotal) / prevTotal;
            }
        }

        DailyRecord record{date, currentCapital, realizedPnL, unrealizedPnL, totalValue, dailyReturn};
        dailyRecords.push_back(record);
    }

    // 运行回测
    void run() {
        for (const Date& date : tradingDates) {
            // 执行策略逻辑（由子类实现）
            executeStrategy(date);

            // 每日结算
            dailySettlement(date);
        }

        // 输出结果
        printResults();

        // 绘制资金曲线
        plotEquityCurve();

        // 输出交易记录
        exportTradeHistory();
    }

    // 策略逻辑（由子类实现）
    virtual void executeStrategy(const Date& date) = 0;

    // 打印结果
    void printResults() {
        std::cout << "\n=== A股回测结果 ===" << std::endl;
        std::cout << "初始资金: " << initialCapital << " 元" << std::endl;
        std::cout << "最终资金: " << currentCapital << " 元" << std::endl;

        double totalReturn = (currentCapital - initialCapital) / initialCapital * 100;
        std::cout << "总收益率: " << std::fixed << std::setprecision(2) << totalReturn << "%" << std::endl;

        if (!dailyRecords.empty()) {
            const DailyRecord& last = dailyRecords.back();
            std::cout << "最终组合价值: " << last.totalValue << " 元" << std::endl;

            // 计算年化收益率
            double years = tradingDates.size() / 245.0; // 近似年化
            if (years > 0) {
                double annualizedReturn = pow(1 + totalReturn/100, 1/years) - 1;
                std::cout << "年化收益率: " << annualizedReturn * 100 << "%" << std::endl;
            }
        }

        // 统计交易
        int filledTrades = 0, rejectedTrades = 0;
        for (const auto& trade : tradeHistory) {
            if (trade.status == "FILLED") filledTrades++;
            else rejectedTrades++;
        }

        std::cout << "\n成交交易: " << filledTrades << " 笔" << std::endl;
        std::cout << "被拒交易: " << rejectedTrades << " 笔" << std::endl;

        // 输出前10笔被拒交易原因
        if (rejectedTrades > 0) {
            std::cout << "\n前10笔被拒交易原因:" << std::endl;
            int count = 0;
            for (const auto& trade : tradeHistory) {
                if (trade.status == "REJECTED" && count < 10) {
                    std::cout << trade.date.toString() << " " << trade.symbol << " " << trade.action
                              << ": " << trade.reason << std::endl;
                    count++;
                }
            }
        }
    }

    // 绘制资金曲线
    void plotEquityCurve() {
        std::ofstream outFile("equity_curve.csv");
        outFile << "日期,组合价值,日收益率\n";

        for (const auto& record : dailyRecords) {
            outFile << record.date.toString() << ","
                    << std::fixed << std::setprecision(2) << record.totalValue << ","
                    << std::fixed << std::setprecision(6) << record.dailyReturn << "\n";
        }

        outFile.close();

        std::cout << "\n资金曲线数据已保存到 equity_curve.csv" << std::endl;
    }

    // 导出交易记录
    void exportTradeHistory() {
        std::ofstream outFile("trade_history.csv");
        outFile << "日期,标的,操作,数量,价格,佣金,状态,原因\n";

        for (const auto& trade : tradeHistory) {
            outFile << trade.date.toString() << ","
                    << trade.symbol << ","
                    << trade.action << ","
                    << std::fixed << std::setprecision(2) << trade.quantity << ","
                    << trade.price << ","
                    << trade.commission << ","
                    << trade.status << ","
                    << trade.reason << "\n";
        }

        outFile.close();

        std::cout << "交易记录已保存到 trade_history.csv" << std::endl;
    }

private:
    // 简单的日期递增
    void incrementDate(Date& date) {
        static const int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};

        date.day++;
        bool isLeap = (date.year % 4 == 0 && date.year % 100 != 0) || (date.year % 400 == 0);

        if (date.month == 2 && isLeap) {
            if (date.day > 29) {
                date.day = 1;
                date.month++;
            }
        } else if (date.day > daysInMonth[date.month - 1]) {
            date.day = 1;
            date.month++;
        }

        if (date.month > 12) {
            date.month = 1;
            date.year++;
        }
    }
};

// 示例策略：双均线策略
class DualMovingAverageStrategy : public AShareBacktestEngine {
private:
    size_t shortWindow = 5;
    size_t longWindow = 20;
    std::map<std::string, std::vector<double>> closePrices;

public:
    DualMovingAverageStrategy(double capital, const Date& start, const Date& end)
        : AShareBacktestEngine(capital, start, end) {}

    void loadPriceData(const std::string& symbol, const std::vector<PriceData>& data) override {
        AShareBacktestEngine::loadPriceData(symbol, data);

        // 存储收盘价用于计算均线
        for (const auto& day : data) {
            closePrices[symbol].push_back(day.close);
        }
    }

    void executeStrategy(const Date& date) override {
        const std::string symbol = "600519"; // 示例股票代码

        // 检查是否有足够数据
        if (closePrices.find(symbol) == closePrices.end() ||
            closePrices[symbol].size() < longWindow) {
            return;
        }

        // 计算短期和长期均线
        double shortMA = calculateMA(symbol, shortWindow);
        double longMA = calculateMA(symbol, longWindow);

        // 获取当前持仓
        bool hasPosition = positions.find(symbol) != positions.end();

        // 交易信号
        if (shortMA > longMA && !hasPosition) {
            // 金叉买入
            double capitalToUse = currentCapital * 0.9; // 使用90%资金
            double price = getCurrentPrice(symbol, date);
            if (price > 0) {
                double quantity = floor(capitalToUse / price / 100) * 100; // A股整手交易
                if (quantity >= 100) { // 至少买1手
                    buy(symbol, quantity, date);
                }
            }
        } else if (shortMA < longMA && hasPosition) {
            // 死叉卖出
            sell(symbol, positions[symbol].quantity, date);
        }
    }

private:
    double calculateMA(const std::string& symbol, size_t window) {
        if (closePrices[symbol].size() < window) return 0.0;

        double sum = 0.0;
        for (size_t i = closePrices[symbol].size() - window; i < closePrices[symbol].size(); ++i) {
            sum += closePrices[symbol][i];
        }
        return sum / window;
    }
};

int main() {
#ifdef _WIN32
    // 设置控制台输出和输入代码页为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    //std::locale::global(std::locale(".65001"));
#endif
    // 创建示例价格数据（贵州茅台）
    std::vector<PriceData> stockData;
    double prevClose = 1600.0;
    for (int i = 1; i <= 60; i++) {
        Date date(2023, 1, i);
        if (date.isTradingDay()) {
            double open = prevClose * (0.99 + (rand() % 30) / 1000.0);
            double close = open * (0.995 + (rand() % 100) / 1000.0);
            double high = std::max(open, close) * (1 + (rand() % 20) / 1000.0);
            double low = std::min(open, close) * (0.99 - (rand() % 20) / 1000.0);
            double upper_limit = round(prevClose * 1.1 * 100) / 100; // 涨停价
            double lower_limit = round(prevClose * 0.9 * 100) / 100; // 跌停价

            // 模拟停牌
            bool suspended = (i == 15 || i == 16); // 第15、16天停牌

            stockData.push_back({
                                    date,
                                    suspended ? 0.0 : open,
                                    suspended ? 0.0 : high,
                                    suspended ? 0.0 : low,
                                    suspended ? 0.0 : close,
                                    suspended ? 0.0 : 1000000,
                                    upper_limit,
                                    lower_limit,
                                    prevClose
                                });

            prevClose = close;
        }
    }

    // 创建并运行回测
    DualMovingAverageStrategy strategy(100000, Date(2023, 1, 1), Date(2023, 3, 31));
    strategy.loadPriceData("600519", stockData);
    strategy.run();

    return 0;
}