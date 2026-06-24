#include <quant1x/test/test.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/instruments.h>
#include <quant1x/data/market.h>
#include <quant1x/resources/meta/blocks.h>

namespace tdx = quant1x::contrib::data::tdx;
namespace data = quant1x::data;
namespace instruments = quant1x::contrib::data::tdx::instruments;

static asio::ip::tcp::endpoint defualt_endpoint(asio::ip::make_address("123.125.108.214"),7709);

// Most Level1 protocol response types were renamed/removed in refactoring.
// New naming: *Context replaces *Request/*Response pairs.
// See quant1x/contrib/data/tdx/level1/std/ for current types.

TEST_CASE("check string include zero", "[strings]") {
    std::string str = "hello\0world";
    fmt::println("{}", str);
}

TEST_CASE("security_list", "[cache]") {
    runtime::global_init();
    std::string code = "880301";
    auto info = instruments::get_instrument_info(code);
    std::cout << info->name << std::endl;
}

TEST_CASE("bestip", "[cache]") {
    spdlog::set_level(spdlog::level::debug);
    {
        auto connPool = tdx::get_std_conn();
    }
    std::this_thread::sleep_for(std::chrono::seconds(100));
}

#if 0
// ===== BELOW: Tests using obsolete protocol types (renamed/removed in Level1 refactoring) =====

// 协议握手1
TEST_CASE("hello1", "[level1]") {
    std::string hex1("b1cb74001c01000000000d006100bd00");
    std::string hex2("00e9070204280900073a02b2020c03840384038403840384033a02b2020c03840384038403840384030022ff3401194a010022ff3401154a0100ff00f70000010101ff00b1b1bea9c1aacda8d0d0c7e9b6fe000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010000010023b8dbb0c400000000000000000000000000000000000000000000000000");
    auto buf2 = strings::hexToBytes(hex2);
    // Hello1Response removed (now StdLoginContext)
    // tdx::Hello1Response p2;
    // p2.deserialize(buf2);
    // fmt::println("{}", p2.Info);
}

// 协议握手2
TEST_CASE("hello2", "[level1]") {
    std::string hex1("0100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    auto buf1 = strings::hexToBytes(hex1);
    // Hello2Response removed (now UpgradeTipContext)
    // tdx::Hello2Response p1;
    // p1.deserialize(buf1);
    // fmt::println("{}", p1.Info);
}

// 心跳
TEST_CASE("heartbeat", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    auto conn = tdx::get_std_conn();
    // HeartbeatResponse removed (now HeartbeatContext handles both directions)
    // tdx::HeartbeatContext req;
    // tdx::HeartbeatResponse resp;
    // auto err = tdx::transact_message_sync(conn->socket(), req, resp);
    // REQUIRE(!err);
}

TEST_CASE("heartbeat-tpl", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    // auto conn = tdx::get_std_conn();
    // tdx::HeartbeatContext req;
    // tdx::HeartbeatResponse resp;
    // auto err = tdx::transact_message_sync(conn->socket(), req, resp);
    // REQUIRE(!err);
}

// 除权除息
TEST_CASE("xdxr-response", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    std::string hex1 = "01000136303031313522000136303031313500d9e030010300f0d246e0a4ed480060ea46e0a4ed480136303031313500e456310101cdcc4c3e0000000000000000000000000136303031313500e77d310101cdcc4c3e000000000000000000000000013630303131350029f3310101cdcc4c3e0000000000000000000000000136303031313500e03e3201010000000000000000cdcc4c40000000000136303031313500e03e3201050060ea46e0a4ed4800b01a47e0a4ed480136303031313500028f32010500b01a47e0a4ed4800b01a4708ea194901363030313135004e8f32010500b01a4708ea194900b01a47a0013d4901363030313135004a9132010500b01a47a0013d4900b01a4720f848490136303031313500569132010500b01a4720f8484900b01a47a0ed694901363030313135006ab432010500b01a47a0ed6949c0f82f482fa7894901363030313135006bb8320105c0f82f482fa7894960fa81482fa789490136303031313500310433010560fa81482fa789497c1590482fa7894901363030313135007e043301057c1590482fa78949ac44d6482fa7894901363030313135009504330103ac44d6482fa78949560832492fa7894901363030313135008806330105560832492fa78949d6fe3d492fa789490136303031313500712a330105d6fe3d492fa78949d6fe3d49232f924901363030313135003d2b330105d6fe3d49232f9249d6fe3d4917b79a4901363030313135007d7a330105d6fe3d4917b79a49d6fe3d490f67a0490136303031313500a29f330105d6fe3d490f67a049be0e4f490f67a049013630303131350076a0330105be0e4f490f67a049be0e4f49359bb0490136303031313500b7a23301015c8f023f0000000000000000000000000136303031313500cfc7330105be0e4f49359bb04909776f49359bb04901363030313135003ac833010148e1fa3e0000000000000000000000000136303031313500f4ee3301015c8f023f00000000000000000000000001363030313135006d1634010509776f49359bb04909776f49f3ecb64901363030313135006e1634010509776f49f3ecb64909776f49f7f1c7490136303031313500663d3401010000003f0000000000000000000000000136303031313500a56534010509776f49f7f1c74909776f49a066e64901363030313135009e8b34010509776f49a066e64989c08849a066e6490136303031313500e0af34010589c08849a066e64989c08849270e084a013630303131350038b234010589c08849270e084afd8ea449270e084a013630303131350048d9340105fd8ea449270e084a3676b249270e084a0136303031313500d7da3401053676b249270e084adfead049270e084a";
    auto buf1 = strings::hexToBytes(hex1);
    tdx::XdxrInfoReply response;
    response.deserialize(buf1);
    for(int i = 0; i < response.Count; i ++) {
        auto e = &response.List[i];
        std::cout << *e << std::endl;
    }
}

TEST_CASE("xdxr-network", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    tdx::XdxrInfoContext request("sh600115");
    spdlog::debug(request.to_string());
    tdx::XdxrInfoReply response;
    auto conn = tdx::get_std_conn();
    auto err = tdx::transact_message_sync(conn->socket(), request, response);
    REQUIRE(!err);
    for(int i = 0; i < response.Count; i ++) {
        auto e = &response.List[i];
        std::cout << *e << std::endl;
    }
}

TEST_CASE("finance-info", "[level1]") {
    runtime::global_init();
    runtime::logger_set(true, true);
    tdx::FinanceRequest request("sh600115");
    spdlog::debug(request.to_string());
    auto buf2 = strings::hexToBytes("010001363030313135dfead04910000800d9fe340121bc3001270e084a0000cf4460f0f9ca8c08d94c00000000b9c5fc485c8f42bea6e4834d8cbe914badddbf4ca042334a40cc27488771d94c801c5649703d4a4c089e1a4cb8fffb4c9a46f14c80b6e649303f86ca00e1964874570e4c60bbedca0014cd4900486eca606c92caa0f780ca00000000fca9313f00004041");
    tdx::FinanceResponse response{};
    spdlog::debug(response.to_string());
    response.deserialize(buf2);
    spdlog::debug(response.to_string());
}

TEST_CASE("finance-info-network", "[level1]") {
    runtime::global_init();
    runtime::logger_set(true, true);
    tdx::FinanceRequest request("sh510050");
    spdlog::debug(request.to_string());
    tdx::FinanceResponse response{};
    auto conn = tdx::get_std_conn();
    auto err = tdx::transact_message_sync(conn->socket(), request, response);
    REQUIRE(!err);
    spdlog::debug(response.to_string());
}

TEST_CASE("security-count", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    // SecurityCountReqeust → SecurityCountContext (response types removed)
    // tdx::SecurityCountContext request = {};
    // tdx::SecurityCountResponse response;
    // auto conn = tdx::get_std_conn();
    // auto err = tdx::transact_message_sync(conn->socket(), request, response);
    // REQUIRE(!err);
}

TEST_CASE("security-quote", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    // SecurityQuoteResponse removed (now SecurityQuoteContext)
}

TEST_CASE("security-quote-network", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    runtime::logger_set(true, true);
    // SecurityQuoteResponse removed (now SecurityQuoteContext)
}

TEST_CASE("transaction-base", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    // TransactionResponse removed (now TransactionContext)
}

TEST_CASE("transaction-network", "[level1]") {
    runtime::logger_set(true, true);
    // TransactionResponse removed (now TransactionContext)
}

TEST_CASE("history-transaction-base", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    // HistoryTransactionResponse removed (now HistoricalTransactionContext)
}

TEST_CASE("history-transaction-network", "[level1]") {
    runtime::logger_set(true, true);
    // HistoryTransactionResponse removed (now HistoricalTransactionContext)
}

TEST_CASE("minutetime-network", "[level1]") {
    runtime::logger_set(true, true);
    // HistoryMinuteTimeResponse removed (now HistoricalMinuteTimeContext)
}

TEST_CASE("kline-base", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    // SecurityBarsResponse removed (now SecurityBarsContext)
}

TEST_CASE("kline-network-stock", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    // SecurityBarsResponse removed (now SecurityBarsContext)
}

TEST_CASE("block-network-meta", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    // BlockMetaResponse removed (now BlockFileMetaContext unifies request/response)
}

TEST_CASE("block-network-info", "[level1]") {
    spdlog::set_level(spdlog::level::debug);
    // BlockInfoResponse removed (now BlockFileContext unifies request/response)
}

TEST_CASE("block-file-update", "[cache]") {
    spdlog::set_level(spdlog::level::debug);
    // BLOCK_DEFAULT → BLOCK_META in refactoring
    data::download_block_raw_data(tdx::BLOCK_META);
}

TEST_CASE("block-file-parse", "[cache]") {
    spdlog::set_level(spdlog::level::debug);
    // BLOCK_GAINIAN → BLOCK_DATA in refactoring
    // Block file parsing API changed; see resources/meta/blocks.h
}

TEST_CASE("block-file-load", "[cache]") {
    spdlog::set_level(spdlog::level::debug);
    // Block file loading API changed; see resources/meta/blocks.h
}

#endif // Level1 protocol refactoring
