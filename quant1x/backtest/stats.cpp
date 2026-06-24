#include <quant1x/backtest/stats.h>

#include <unordered_map>
#include <deque>
#include <algorithm>
#include <cmath>

namespace backtest {

void computeRoundTripStatsFromTrades(const std::vector<Trade> &trades, BacktestResult &result) {
    size_t closed_roundtrips = 0;
    size_t winning_roundtrips = 0;
    double total_profit = 0.0;
    double total_loss = 0.0;

    struct OpenLot {
        double quantity;
        double price;
        double realized_pnl;
        explicit OpenLot(double q = 0.0, double p = 0.0) : quantity(q), price(p), realized_pnl(0.0) {}
    };

    std::unordered_map<std::string, std::deque<OpenLot>> long_opens;
    std::unordered_map<std::string, std::deque<OpenLot>> short_opens;

    const double EPS = 1e-12;
    for (const auto &trd : trades) {
        const std::string &sym = trd.symbol;
        double qty = static_cast<double>(trd.quantity);
        double price = trd.price;

        if (trd.direction == TradeDirection::LONG) {
            // First, try to close short opens (cover)
            double need = qty;
            auto &sq = short_opens[sym];
            while (need > 0.0 && !sq.empty()) {
                OpenLot &open = sq.front();
                double m = std::min(need, open.quantity);
                // For short-open then long-close, pnl = open.price - close_price
                double pnl = (open.price - price) * m;
                open.realized_pnl += pnl;
                need -= m;

                if (m >= open.quantity - EPS) {
                    if (open.realized_pnl > 0.0) {
                        ++winning_roundtrips;
                        total_profit += open.realized_pnl;
                    } else {
                        total_loss += std::abs(open.realized_pnl);
                    }
                    ++closed_roundtrips;
                    sq.pop_front();
                } else {
                    open.quantity -= m;
                }
            }
            // Any remaining quantity becomes a new long open
            if (need > 0.0) {
                long_opens[sym].push_back(OpenLot(need, price));
            }
        } else if (trd.direction == TradeDirection::SHORT) {
            // First, try to close long opens
            double need = qty;
            auto &lq = long_opens[sym];
            while (need > 0.0 && !lq.empty()) {
                OpenLot &open = lq.front();
                double m = std::min(need, open.quantity);
                // For long-open then short-close, pnl = close_price - open.price
                double pnl = (price - open.price) * m;
                open.realized_pnl += pnl;
                need -= m;

                if (m >= open.quantity - EPS) {
                    if (open.realized_pnl > 0.0) {
                        ++winning_roundtrips;
                        total_profit += open.realized_pnl;
                    } else {
                        total_loss += std::abs(open.realized_pnl);
                    }
                    ++closed_roundtrips;
                    lq.pop_front();
                } else {
                    open.quantity -= m;
                }
            }
            // Any remaining becomes a new short open
            if (need > 0.0) {
                short_opens[sym].push_back(OpenLot(need, price));
            }
        }
    }

    // 填充结果: 已完成的 round-trip 个数
    result.closed_trades = closed_roundtrips;
    result.closed_roundtrips = closed_roundtrips;
    // 向后兼容的旧字段
    result.total_trades = closed_roundtrips;
    // 原子级成交事件数(每笔成交/fill)
    result.trade_events_count = trades.size();
    result.winning_trades = winning_roundtrips;
    result.losing_trades = (closed_roundtrips > winning_roundtrips) ? (closed_roundtrips - winning_roundtrips) : 0;
    result.win_rate = closed_roundtrips > 0 ? (static_cast<double>(winning_roundtrips) / static_cast<double>(closed_roundtrips) * 100.0) : 0.0;

    result.avg_profit = (winning_roundtrips > 0) ? (total_profit / static_cast<double>(winning_roundtrips)) : 0.0;
    result.avg_loss = (result.losing_trades > 0) ? (total_loss / static_cast<double>(result.losing_trades)) : 0.0;
    result.profit_loss_ratio = (result.avg_loss != 0.0) ? (result.avg_profit / result.avg_loss) : 0.0;
}

} // namespace backtest
