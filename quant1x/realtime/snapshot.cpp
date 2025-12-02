#include <quant1x/realtime/snapshot.h>
#include <quant1x/exchange/exchange.h>
#include <quant1x/std/util.h>
#include <capnp/serialize.h>
#include <capnp/message.h>
#include <mio/mmap.hpp>
#include <quant1x/proto/data.h>
#include <quant1x/runtime/scheduler.h>

namespace realtime {

    /**
     * @brief 从 SecurityQuote 中提取 imbalance 指标
     * @param quote 当前行情快照
     * @return ImbalanceResult 包含 imbalance 指标
     */
    ImbalanceResult calculateImbalance(const level1::SecurityQuote& quote) {
        // 买盘挂单量
        std::vector<int64_t> bidVolumes = {
            quote.bidVol1, quote.bidVol2, quote.bidVol3,
            quote.bidVol4, quote.bidVol5
        };

        // 卖盘挂单量
        std::vector<int64_t> askVolumes = {
            quote.askVol1, quote.askVol2, quote.askVol3,
            quote.askVol4, quote.askVol5
        };

        // 买盘价格
        std::vector<double> bidPrices = {
            quote.bid1, quote.bid2, quote.bid3,
            quote.bid4, quote.bid5
        };

        // 卖盘价格
        std::vector<double> askPrices = {
            quote.ask1, quote.ask2, quote.ask3,
            quote.ask4, quote.ask5
        };

        // 总挂单量
        int64_t totalBidVol = 0;
        int64_t totalAskVol = 0;

        // 加权挂单量（price × volume）
        double weightedBid = 0.0;
        double weightedAsk = 0.0;

        for (int i = 0; i < 5; ++i) {
            totalBidVol += bidVolumes[i];
            totalAskVol += askVolumes[i];

            weightedBid += bidPrices[i] * double(bidVolumes[i]);
            weightedAsk += askPrices[i] * double(askVolumes[i]);
        }

        // 简单 imbalance：(bid - ask) / (bid + ask)
        double simpleImbalance = 0.0;
        if (totalBidVol + totalAskVol > 0) {
            simpleImbalance = static_cast<double>(totalBidVol - totalAskVol) /
                              static_cast<double>(totalBidVol + totalAskVol);
        }

        // 加权 imbalance
        double weightedImbalance = 0.0;
        if (std::abs(weightedBid + weightedAsk) > 1e-9) {
            weightedImbalance = (weightedBid - weightedAsk) / (weightedBid + weightedAsk);
        }

        return {simpleImbalance, weightedImbalance};
    }


    static const std::string capnp_cache_filename = config::default_cache_path() + "/cache/" + "quote_list.capnp";
    static constexpr const size_t capnp_cache_size = 64 * 1024 * 1024; // 64MB
    namespace fs = std::filesystem;

    void ensure_file_size(const std::string &path, size_t required_size) {
        // 检查文件是否存在
        if (fs::exists(path)) {
            // 获取当前大小
            size_t current_size = fs::file_size(path);

            if (current_size > required_size) {
                spdlog::error("[snapshot] File already exists and is large enough.");
                return;
            } else if (current_size == required_size) {
                return;
            }
            //std::cout << "File exists but too small (" << current_size << " bytes). Resizing to " << required_size << " bytes.\n";
        } else {
            //std::cout << "File does not exist. Creating new file with size " << required_size << " bytes.\n";
        }

        // 打开文件进行截断/扩展
        std::ofstream file(path, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) {
            std::ofstream create_file(path, std::ios::out | std::ios::binary);
            if (!create_file) {
                throw std::runtime_error("[snapshot] 无法创建文件: " + path);
            }
            create_file.close();
            file.open(path, std::ios::in | std::ios::out | std::ios::binary);
        }

        // 移动指针到指定位置，并写入一个字节触发扩容
        file.seekp(required_size - 1);
        file.write("\0", 1);

        if (!file) {
            throw std::runtime_error("[snapshot] 文件扩展失败: " + path);
        }

        //std::cout << "File resized successfully.\n";
    }

    namespace {
        inline tsl::robin_map<std::string, level1::SecurityQuote> mem_snapshots;
        inline std::shared_mutex                                  mem_mutex;
    }

    void sync_snapshots() {
        auto all_codes = exchange::GetCodeList();
        auto count = all_codes.size();
        util::check_filepath(capnp_cache_filename, true);
        // 确保文件存在且大小合适
        ensure_file_size(capnp_cache_filename, capnp_cache_size);

        // Read-write memory map the whole file by using `map_entire_file` where the
        // length of the mapping is otherwise expected, with the factory method.
        std::error_code error;
        mio::mmap_sink rw_mmap = mio::make_mmap_sink(capnp_cache_filename, 0, mio::map_entire_file, error);
        if (error) {
            const auto &errmsg = error.message();
            spdlog::error("[snapshot] error mapping file: {}, exiting...", errmsg.c_str());
            return;
        }

        // 获取 mmap 数据指针
        char *base = reinterpret_cast<char *>(rw_mmap.data());
        size_t capacity = rw_mmap.size();
        spdlog::info("[snapshot] capacity = {}", capacity);

        // 创建一个消息构建器
        capnp::MallocMessageBuilder message;
        auto quoteList = message.initRoot<QuoteList>();
        //auto quoteList = builder.initRoot<QuoteList>();
        auto snapshots = quoteList.initSnapshots(uint32_t(count));
        size_t start = 0;
        auto tp_start = std::chrono::high_resolution_clock::now();
        auto last_trade_day = exchange::last_trading_day();
        auto current_day = last_trade_day.only_date();
        auto [update_in_realTime, status] = exchange::can_update_in_realtime(exchange::timestamp::now());
        try {
            spdlog::warn("[snapshot] start = {}", exchange::timestamp::now().toString());
            for (; start < count; start += level1::security_quotes_max) {
                std::unique_lock lock(mem_mutex);
                auto length = count - start;
                if (length > level1::security_quotes_max) {
                    length = level1::security_quotes_max;
                }
                std::vector<std::string> sub_codes(all_codes.begin() + start, all_codes.begin() + start + length);
                tsl::robin_map<std::string, level1::StockInfo> maps;
                maps.clear();
                size_t i = 0;
                for (; i < length; i++) {
                    const auto &code = sub_codes[i];
                    auto [mid, mflag, symbol] = exchange::DetectMarket(code);
                    maps[code] = level1::StockInfo{mid, symbol};
                }
                level1::SecurityQuoteRequest request(sub_codes);
                level1::SecurityQuoteResponse response;
                auto conn = level1::get_std_conn();
                if(conn == nullptr) {
                    spdlog::error("服务器网络不稳定, 稍后重试");
                    return;
                }
                auto err = level1::process(conn->socket(), request, response);
                if (err) {
                    spdlog::error("Process error: {}", err.message());
                    return;
                }
                response.verify_delisted_securities(maps);
                for (int j = 0; j < response.count; ++j) {
                    const auto &raw = response.list[j];
                    std::string security_code = exchange::GetSecurityCode(static_cast<exchange::MarketType>(raw.market), raw.code);
                    mem_snapshots.insert_or_assign(security_code, raw);
                    auto snap = snapshots[uint32_t(start) + j];
                    snap.setDate(current_day);
                    snap.setSecurityCode(security_code);
                    auto exchangeState = ExchangeState::CLOSING;
                    if (raw.state == level1::TradeState::DELISTING) {
                        exchangeState = ExchangeState::DELISTING;
                    } else if (raw.state == level1::TradeState::SUSPEND) {
                        exchangeState = ExchangeState::PAUSE;
                    }
                    if (update_in_realTime) {
                        exchangeState = ExchangeState::NORMAL;
                    }
                    if (status == exchange::TimeStatus::ExchangeHaltTrading) {
                        exchangeState = ExchangeState::NORMAL;
                    }
                    snap.setExchangeState(exchangeState);
                    auto stockState = TradeState::DELISTING;
                    if(raw.state == level1::TradeState::SUSPEND) {
                        stockState = TradeState::SUSPEND;
                    } else if(raw.state == level1::TradeState::NORMAL) {
                        stockState = TradeState::NORMAL;
                    } else if(raw.state == level1::TradeState::IPO) {
                        stockState = TradeState::IPO;
                    }
                    snap.setState(stockState);
                    snap.setMarket(raw.market); // market: 0 or 1
                    snap.setCode(raw.code);
                    snap.setActive(raw.active1);
                    snap.setPrice(raw.price);
                    snap.setLastClose(raw.lastClose);
                    snap.setOpen(raw.open);
                    snap.setHigh(raw.high);
                    snap.setLow(raw.low);
                    snap.setServerTime(raw.serverTime);
                    snap.setReversedBytes0(raw.reversedBytes0);
                    snap.setReversedBytes1(raw.reversedBytes1);
                    snap.setVol(raw.vol);
                    snap.setCurVol(raw.curVol);
                    snap.setAmount(raw.amount);
                    snap.setSVol(raw.sVol);
                    snap.setBVol(raw.bVol);
                    snap.setIndexOpenAmount(raw.indexOpenAmount);
                    snap.setStockOpenAmount(raw.stockOpenAmount);
                    snap.setOpenVolume(raw.openVolume);
                    snap.setCloseVolume(raw.closeVolume);
                    snap.setIndexUp(raw.indexUp);
                    snap.setIndexUpLimit(raw.indexUpLimit);
                    snap.setIndexDown(raw.indexDown);
                    snap.setIndexDownLimit(raw.indexDownLimit);
                    snap.setBid1(raw.bid1);
                    snap.setAsk1(raw.ask1);
                    snap.setBidVol1(raw.bidVol1);
                    snap.setAskVol1(raw.askVol1);
                    snap.setBid2(raw.bid2);
                    snap.setAsk2(raw.ask2);
                    snap.setBidVol2(raw.bidVol2);
                    snap.setAskVol2(raw.askVol2);
                    snap.setBid3(raw.bid3);
                    snap.setAsk3(raw.ask3);
                    snap.setBidVol3(raw.bidVol3);
                    snap.setAskVol3(raw.askVol3);
                    snap.setBid4(raw.bid4);
                    snap.setAsk4(raw.ask4);
                    snap.setBidVol4(raw.bidVol4);
                    snap.setAskVol4(raw.askVol4);
                    snap.setBid5(raw.bid5);
                    snap.setAsk5(raw.ask5);
                    snap.setBidVol5(raw.bidVol5);
                    snap.setAskVol5(raw.askVol5);
                    snap.setReversedBytes4(raw.reversedBytes4);
                    snap.setReversedBytes5(raw.reversedBytes5);
                    snap.setReversedBytes6(raw.reversedBytes6);
                    snap.setReversedBytes7(raw.reversedBytes7);
                    snap.setReversedBytes8(raw.reversedBytes8);
                    snap.setRate(raw.rate);
                    snap.setActive2(raw.active2);
                    snap.setTimeStamp(raw.timeStamp);
                }
                //spdlog::warn("code range: {}=>{}, end", start, start+length);
            }
            // 序列化为 flat array
            auto flat_array = capnp::messageToFlatArray(message);
            size_t data_size = flat_array.size() * sizeof(capnp::word);
            // 拷贝 Cap'n Proto 数据
            memcpy(reinterpret_cast<char *>(base), flat_array.begin(), data_size);
            auto tp_end = std::chrono::high_resolution_clock::now();
            auto diff = tp_end - tp_start;
            //std::cout << diff << std::endl;
            spdlog::warn("[snapshot] stop = {}", exchange::timestamp::now().toString());
            spdlog::info("[snapshot] cross time:{}", util::format_duration_auto(diff));
        } catch (const std::exception &e) {  // 其他标准异常
            spdlog::error("[snapshot] - 标准异常: {} (type: {})", e.what(), typeid(e).name());
            // 对于system_error可以记录更多信息
            if (auto se = dynamic_cast<const std::system_error *>(&e)) {
                spdlog::error("[snapshot] Error code: {}, category: {}", se->code().value(), se->code().category().name());
            }
        } catch (...) {
            spdlog::error("[snapshot] 获取快照异常");
        }
    }

    std::optional<level1::SecurityQuote> get_snapshot(const std::string &code) {
        std::shared_lock lock(mem_mutex);
        auto it = mem_snapshots.find(code);
        if (it != mem_snapshots.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    namespace {
        static inline tsl::robin_map<std::string, Snapshot::Reader> cache_snapshots;
        static inline std::shared_ptr<mio::mmap_sink>               cache_snapshot_mmap;
        static inline std::unique_ptr<capnp::FlatArrayMessageReader> cache_reader;
        static inline std::mutex                                     cache_mutex;
    }

    void load_snapshots() {
        std::error_code error;
        auto new_mmap = std::make_shared<mio::mmap_sink>();
        *new_mmap = mio::make_mmap_sink(capnp_cache_filename, 0, mio::map_entire_file, error);
        if (error) {
            const auto& errmsg = error.message();
            spdlog::error("[realtime-snapshot] error mapping file: {}, exiting...", errmsg.c_str());
            return;
        }

        // 3. 解析 Cap'n Proto 数据
        try {
            auto base = reinterpret_cast<const capnp::word*>(new_mmap->data());
            size_t word_count = new_mmap->size() / sizeof(capnp::word);
            kj::ArrayPtr<const capnp::word> words(base, word_count);

            auto new_reader = std::make_unique<capnp::FlatArrayMessageReader>(words);
            auto quoteList = new_reader->getRoot<QuoteList>();

            // 4. 更新缓存（加锁）
            {
                std::lock_guard<std::mutex> lock(cache_mutex);
                cache_snapshots.clear();
                if(cache_snapshot_mmap != nullptr) {
                    cache_snapshot_mmap->unmap();
                }
                cache_snapshot_mmap = new_mmap; // 保持 mmap 存活
                cache_reader = std::move(new_reader); // 转移 reader 所有权
                for (auto v : quoteList.getSnapshots()) {
                    cache_snapshots.insert_or_assign(v.getSecurityCode(), v);
                }
            }
            spdlog::info("[realtime-snapshot] Reloaded snapshot successfully");
        } catch (const kj::Exception& e) {
            spdlog::error("[realtime-snapshot] Cap'n Proto parse error: {}", e.getDescription().cStr());
        }
    }

    std::optional<Snapshot::Reader> snapshot(const std::string &code) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache_snapshots.find(code);
        if (it != cache_snapshots.end()) {
            return it->second;
        }
        return std::nullopt;
    }


} // namespace realtime