#include <quant1x/test/test.h>
#include <quant1x/runtime/config.h>
#include <quant1x/markets/instruments.h>
#include <quant1x/std/util.h>

TEST_CASE("base-snapshot", "[runtime]") {
    runtime::global_init();
    runtime::logger_set(false, false);
    auto all_codes = instruments::get_code_list();
    //std::span<std::string> codes(all_codes);
    auto count = all_codes.size();
    size_t start = 0;
    auto tp_start = std::chrono::high_resolution_clock::now();
    try {
        spdlog::warn("start = {}", meta::Timestamp::now().toString());
        for (; start < count; start += tdx::security_quotes_max) {
            auto length = count - start;
            if (length > tdx::security_quotes_max) {
                length = tdx::security_quotes_max;
            }
            //auto sub_codes = all_codes.subspan(start, length);
            spdlog::warn("code range: {}=>{}, begin", start, start+length);
            std::vector<std::string> sub_codes(all_codes.begin() + start, all_codes.begin() + start + length);
            tsl::robin_map<std::string, tdx::StockInfo> maps;
            maps.clear();
            size_t i = 0;
            for (; i < length; i++) {
                const auto& code = sub_codes[i];
                auto [mid, mflag, symbol] = data::detect_symbol(code);
                maps[code] = tdx::StockInfo{mid, symbol};
            }
            tdx::SecurityQuoteContext request(sub_codes);
            tdx::SecurityQuoteResponse response;
            auto conn = tdx::get_std_conn();
            auto err = tdx::transact_message_sync(conn->socket(), request, response);
            REQUIRE(!err);
            response.verify_delisted_securities(maps);
            spdlog::warn("code range: {}=>{}, end", start, start+length);
        }
        auto tp_end = std::chrono::high_resolution_clock::now();
        auto diff = tp_end - tp_start;
        //std::cout << diff << std::endl;
        spdlog::warn("stop = {}", meta::Timestamp::now().toString());
        spdlog::info("cross time:{}", util::format_duration_auto(diff));
    } catch (const std::exception &e) {  // 其他标准异常
        spdlog::error("全局捕获 - 标准异常: {} (type: {})", e.what(), typeid(e).name());
        // 对于system_error可以记录更多信息
        if (auto se = dynamic_cast<const std::system_error *>(&e)) {
            spdlog::error("Error code: {}, category: {}", se->code().value(), se->code().category().name());
        }
    } catch (...) {
        spdlog::error("获取日K线异常");
    }
}

#include <quant1x/proto/data.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>

TEST_CASE("capnp-snapshot", "[runtime]") {
    runtime::logger_set(true, true);
    capnp::MallocMessageBuilder message;
    auto quoteList = message.initRoot<QuoteList>();
    auto snapshots = quoteList.initSnapshots(2);
    // 第一个快照
    snapshots[0].setSecurityCode("SH600000");
    snapshots[0].setPrice(30.5);
    snapshots[0].setExchangeState(ExchangeState::NORMAL);

    // 第二个快照
    snapshots[1].setSecurityCode("SZ000001");
    snapshots[1].setPrice(15.2);
    snapshots[1].setExchangeState(ExchangeState::PAUSE);

    // 序列化
    kj::Array<capnp::word> bytes = capnp::messageToFlatArray(message);
    std::cout << "QuoteList size: " << bytes.size() * sizeof(capnp::word) << " bytes\n";

    // 反序列化
    capnp::FlatArrayMessageReader reader(bytes);
    auto quoteList1 = reader.getRoot<QuoteList>();
    auto snapshots1 = quoteList1.getSnapshots();

    for (const auto& snap : snapshots1) {
        std::cout << "Security Code: " << snap.getSecurityCode().cStr()
                  << ", Price: " << snap.getPrice()
                  //<< ", Exchange State: " << snap.getExchangeState()
                  << "\n";
    }
}

void saveToFile(const kj::Array<capnp::word>& data, const std::string& filename) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    // 写入字节流
    file.write(reinterpret_cast<const char*>(data.begin()), data.size() * sizeof(capnp::word));
    spdlog::info("Saved serialized data to {}, size: {} bytes", filename, data.size() * sizeof(capnp::word));
}

kj::Array<capnp::word> readFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for reading: " << filename << "\n";
        return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        size_t wordCount = size / sizeof(capnp::word);

        // 使用 heapArray 分配新内存并拷贝数据
        auto words = kj::heapArray<capnp::word>(wordCount);
        memcpy(words.begin(), buffer.data(), wordCount * sizeof(capnp::word));

        return words;  // 返回拥有所有权的 Array<capnp::word>
    }

    return nullptr;
}

constexpr const char * const tdd_capnp_cache_filename = "quote_list.capnp";
constexpr size_t tdd_capnp_cache_size = 64 * 1024 * 1024; // 64MB
namespace fs = std::filesystem;

#include <mio/mmap.hpp>

void ensure_file_size(const std::string& path, size_t required_size) {
    // 检查文件是否存在
    if (fs::exists(path)) {
        // 获取当前大小
        size_t current_size = fs::file_size(path);

        if (current_size >= required_size) {
            std::cout << "File already exists and is large enough.\n";
            return;
        }
        std::cout << "File exists but too small (" << current_size << " bytes). Resizing to " << required_size << " bytes.\n";
    } else {
        std::cout << "File does not exist. Creating new file with size " << required_size << " bytes.\n";
    }

    // 打开文件进行截断/扩展
    std::ofstream file(path, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        std::ofstream create_file(path, std::ios::out | std::ios::binary);
        if (!create_file) {
            throw std::runtime_error("无法创建文件: " + path);
        }
        create_file.close();
        file.open(path, std::ios::in | std::ios::out | std::ios::binary);
    }

    // 移动指针到指定位置, 并写入一个字节触发扩容
    file.seekp(required_size - 1);
    file.write("\0", 1);

    if (!file) {
        throw std::runtime_error("文件扩展失败: " + path);
    }

    std::cout << "File resized successfully.\n";
}

TEST_CASE("test-buffer-size", "[capnp]") {
    runtime::global_init();
    runtime::logger_set(false, false);
    auto all_codes = instruments::get_code_list();
    auto count = all_codes.size();
    capnp::MallocMessageBuilder message;
    auto quoteList = message.initRoot<QuoteList>();
    auto snapshots = quoteList.initSnapshots(uint32_t(count));
    (void)snapshots;
    // 序列化
    kj::Array<capnp::word> bytes = capnp::messageToFlatArray(message);
    saveToFile(bytes, tdd_capnp_cache_filename);
}

#include <capnp/serialize.h>
#include <capnp/message.h>

TEST_CASE("tick-snapshot", "[runtime]") {
    runtime::global_init();
    runtime::logger_set(false, false);
    auto all_codes = instruments::get_code_list();
    auto count = all_codes.size();

    // 确保文件存在且大小合适
    ensure_file_size(tdd_capnp_cache_filename, tdd_capnp_cache_size);

    // Read-write memory map the whole file by using `map_entire_file` where the
    // length of the mapping is otherwise expected, with the factory method.
    std::error_code error;
    mio::mmap_sink rw_mmap = mio::make_mmap_sink(tdd_capnp_cache_filename, 0, mio::map_entire_file, error);
    if (error) {
        const auto& errmsg = error.message();
        spdlog::error("error mapping file: {}, exiting...", errmsg.c_str());
        return;
    }

    // 获取 mmap 数据指针
    char* base = reinterpret_cast<char*>(rw_mmap.data());
    size_t capacity = rw_mmap.size();
    spdlog::info("capacity = {}", capacity);
//    // 创建一个输出流指向这块内存
//    kj::ArrayOutputStream outputStream(kj::ArrayPtr<kj::byte>(
//        reinterpret_cast<kj::byte*>(base),
//        capacity
//    ));
//    // 转换为 Cap'n Proto 可用的内存块(按 word 对齐)
//    kj::ArrayPtr<capnp::word> mmapWords(reinterpret_cast<capnp::word*>(base), capacity / sizeof(capnp::word));
//    // 使用 FlatMessageBuilder 在 mmap 内存上构建消息
//    capnp::FlatMessageBuilder builder(mmapWords);
//    //builder.initRoot<QuoteList, QuoteList::brand()>();

    // 创建一个消息构建器
    capnp::MallocMessageBuilder message;
    auto quoteList = message.initRoot<QuoteList>();
    //auto quoteList = builder.initRoot<QuoteList>();
    auto snapshots = quoteList.initSnapshots(uint32_t(count));
    size_t start = 0;
    auto tp_start = std::chrono::high_resolution_clock::now();
    auto last_trade_day = meta::last_trading_day();
    auto current_day = last_trade_day.only_date();
    auto [update_in_realTime, status] = false(meta::Timestamp::now());
    try {
        spdlog::warn("start = {}", meta::Timestamp::now().toString());
        for (; start < count; start += tdx::security_quotes_max) {
            auto length = count - start;
            if (length > tdx::security_quotes_max) {
                length = tdx::security_quotes_max;
            }
            //auto sub_codes = all_codes.subspan(start, length);
            //spdlog::warn("code range: {}=>{}, begin", start, start+length);
            std::vector<std::string> sub_codes(all_codes.begin() + start, all_codes.begin() + start + length);
            tsl::robin_map<std::string, tdx::StockInfo> maps;
            maps.clear();
            size_t i = 0;
            for (; i < length; i++) {
                const auto& code = sub_codes[i];
                auto [mid, mflag, symbol] = data::detect_symbol(code);
                maps[code] = tdx::StockInfo{mid, symbol};
            }
            tdx::SecurityQuoteContext request(sub_codes);
            tdx::SecurityQuoteResponse response;
            auto conn = tdx::get_std_conn();
            auto err = tdx::transact_message_sync(conn->socket(), request, response);
            REQUIRE(!err);
            response.verify_delisted_securities(maps);
            for (int j = 0; j < response.count; ++j) {
                const auto & raw = response.list[j];
                auto snap = snapshots[uint32_t(start)+j];
                snap.setDate(current_day);
                snap.setSecurityCode(data::correct_security_code(static_cast<meta::InstrumentType>(raw.market), raw.code));
                auto exchangeState = ExchangeState::CLOSING;
                if(raw.state == tdx::TradeState::DELISTING) {
                    exchangeState = ExchangeState::DELISTING;
                } else if (raw.state == tdx::TradeState::SUSPEND) {
                    exchangeState= ExchangeState::PAUSE;
                }
                if (update_in_realTime) {
                    exchangeState = ExchangeState::NORMAL;
                }
                if (status == meta::TimeStatus::ExchangeHaltTrading) {
                    exchangeState = ExchangeState::NORMAL;
                }
                snap.setExchangeState(exchangeState);
                snap.setState(raw.state == tdx::TradeState::DELISTING ? TradeState::DELISTING : TradeState::NORMAL);
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
        // 序列化
        //kj::Array<capnp::word> bytes = capnp::messageToFlatArray(message);
        //saveToFile(bytes, tdd_capnp_cache_filename);
        //capnp::writeMessage(outputStream, message);
        //rw_mmap.sync(error);
        //rw_mmap.unmap();

        // 序列化为 flat array
        auto flat_array = capnp::messageToFlatArray(message);
        size_t data_size = flat_array.size() * sizeof(capnp::word);
        // 拷贝 Cap'n Proto 数据
        memcpy(reinterpret_cast<char*>(base), flat_array.begin(), data_size);

        //capnp::messageToFlatArray(builder);

        auto tp_end = std::chrono::high_resolution_clock::now();
        auto diff = tp_end - tp_start;
        //std::cout << diff << std::endl;
        spdlog::warn("stop = {}", meta::Timestamp::now().toString());
        spdlog::info("cross time:{}", util::format_duration_auto(diff));
    } catch (const std::exception &e) {  // 其他标准异常
        spdlog::error("全局捕获 - 标准异常: {} (type: {})", e.what(), typeid(e).name());
        // 对于system_error可以记录更多信息
        if (auto se = dynamic_cast<const std::system_error *>(&e)) {
            spdlog::error("Error code: {}, category: {}", se->code().value(), se->code().category().name());
        }
    } catch (...) {
        spdlog::error("获取日K线异常");
    }
}

#include <capnp/compat/json.h>        // JSON 编码头文件

static void printBinaryFileAsJson(const std::string &filename) {
    // Read-write memory map the whole file by using `map_entire_file` where the
    // length of the mapping is otherwise expected, with the factory method.
    std::error_code error;
    mio::mmap_sink rw_mmap = mio::make_mmap_sink(filename, 0, mio::map_entire_file, error);
    if (error) {
        const auto& errmsg = error.message();
        spdlog::error("error mapping file: {}, exiting...", errmsg.c_str());
        return;
    }

    // 获取 mmap 数据指针
    char* base = reinterpret_cast<char*>(rw_mmap.data());
    size_t capacity = rw_mmap.size();
    spdlog::info("capacity = {}", capacity);

    // 转换为 Cap'n Proto 可识别的 word 数组
    kj::ArrayPtr<const capnp::word> words(
        reinterpret_cast<const capnp::word*>(base),
        capacity / sizeof(capnp::word)
    );

    // 解析消息
    capnp::FlatArrayMessageReader reader(words);
    QuoteList::Reader quoteList = reader.getRoot<QuoteList>();
    //auto snapshots = quoteList.getSnapshots();
    // 转换为 JSON 并打印
    capnp::JsonCodec jsonCodec;
    kj::String json = jsonCodec.encode(quoteList);
    std::cout << json.cStr() << std::endl;
    spdlog::debug(json.cStr());
}

TEST_CASE("print-snapshot", "[capnp]") {
    runtime::global_init();
    runtime::logger_set(true, true);
    printBinaryFileAsJson(tdd_capnp_cache_filename);
}

#include <quant1x/realtime/snapshot.h>
#include <quant1x/trader/tracker.h>
#include <users/no1.h>

TEST_CASE("sync-snapshot", "[realtime]") {
    realtime::sync_snapshots();
}

TEST_CASE("get-snapshot", "[realtime]") {
    runtime::global_init();
    realtime::load_snapshots();
    std::string code = "600600";
    auto security_code = data::correct_security_code(code);
    auto ss = realtime::snapshot(security_code);
    if(ss.has_value()) {
        std::string ts = ss->getTimeStamp();
        std::cout << uint64_t(ss->getState()) << std::endl;
        std::cout << ts << std::endl;
        std::cout << ss->getSecurityCode().cStr() << std::endl;
    } else {
        std::cout << "not found" << std::endl;
    }
}

TEST_CASE("tracker-no1", "[realtime]") {
    runtime::global_init();
    runtime::logger_set(true, true);
    auto const & config = config::TraderConfig();
    realtime::sync_snapshots();
    // 注册策略
    StrategyManager& manager = StrategyManager::Instance();
    StrategyPtr s1 = std::make_shared<HousNo1Strategy>();
    manager.Register(s1);

    // 打印配置
    std::cout << *config << std::endl;
    trader::tracker();
    trader::tracker();
}

TEST_CASE("tracker-no1-in-trading", "[realtime]") {
    runtime::global_init();
    auto const & config = config::TraderConfig();
    uint64_t strategyId = 1; // 1号策略
    auto strategyParameter = config->GetStrategyParameterByCode(strategyId);
    std::cout << strategyParameter.value() << std::endl;
    std::cout << "--------------------" << std::endl;
    std::cout << strategyParameter.value().Session.IsTrading() << std::endl;
}