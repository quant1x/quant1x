#include <quant1x/exchange/blocks.h>
#include <quant1x/io/file.h>
#include <quant1x/runtime/config.h>
#include <quant1x/exchange/session.h>
#include <quant1x/level1/client.h>
#include <quant1x/resources/meta/blocks.h>

namespace exchange {

    std::ostream &operator<<(std::ostream &os, const block_info &info) {
        os << "{code:" << info.code
           << ", name:" << info.name
           << ", type:" << info.type
           << ", num:" << info.num
           << ", ConstituentStocks:[";
        bool first = true;
        for(auto const & v : info.ConstituentStocks) {
            if (!first) {
                os << ",";
            } else {
                first = false;
            }
            os << v;
        }
        os << "]}";
        return os;
    }

    namespace fs = std::filesystem;

    // 下载板块原数据
    void download_block_raw_data(const std::string &filename) {
        auto blkFilename = config::get_block_path() + "/" + filename;
        bool needUpdate = false;
        if (!fs::exists(blkFilename) || fs::file_size(blkFilename) == 0) {
            needUpdate = true;
        }
        auto modified = io::last_modified_time(blkFilename);
        if(!needUpdate) {
            needUpdate = exchange::can_initialize(modified);
        }
        if(needUpdate) {
            util::check_filepath(blkFilename, true);
            std::ofstream file(blkFilename, std::ios::binary|std::ios::out|std::ios::trunc);
            if(!file.is_open()) {
                spdlog::error("[exchange::blocks] Failed to open file: {}", filename);
                return;
            }
            spdlog::debug("[exchange::blocks] open file: {}", filename);
            for(u32 start = 0;;) {
                level1::BlockInfoRequest request(filename, start);
                level1::BlockInfoResponse response;
                auto conn = level1::client();
                level1::process(conn->socket(), request, response);
                auto data = response.Data;
                if( response.Size > 0) {
                    file.write(reinterpret_cast<const char *>(data.data()), response.Size);
                }
                if(response.Size < level1::BLOCK_CHUNKS_SIZE) {
                    break;
                }
                start+= response.Size;
            }
            file.close();
            spdlog::debug("[exchange::blocks] close file: {}", filename);
        }
    }

    // 解析板块文件数据
    std::vector<block_info> parse_block_raw_data(const std::string &filename) {
        auto blkFilename = config::get_block_path() + "/" + filename;
        std::ifstream in(blkFilename, std::ios::binary);
        if(!in.is_open()) {
            spdlog::error("[exchange::blocks] 板块文件[{}], 打开失败", blkFilename);
            return {};
        }
        std::vector<uint8_t> buf((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        BinaryStream bs(buf);
        bs.skip(384);
        u16 Count = bs.get_u16();
        std::vector<block_info> list;
        for (int i = 0; i < Count; i++) {
            block_info bi{};
            u8 tmpBuf1[2813] = {0};
            bs.get_array(tmpBuf1);
            BinaryStream bs1(tmpBuf1);
            std::string tmp = bs1.get_string(9);
            bi.name = charsets::gbk_to_utf8(tmp);
            bi.num = bs1.get_u16();
            bi.type = bs1.get_u16();
            u8 tmpBuf2[400*7];
            bs1.get_array(tmpBuf2);
            BinaryStream bs2(tmpBuf2);
            bi.ConstituentStocks.resize(bi.num); // 成分股
            for (int j = 0; j < bi.num; j++) {
                std::string symbol = bs2.get_string(7);
                bi.ConstituentStocks[j] = symbol;
            }
            list.emplace_back(bi);
        }
        in.close();
        return list;
    }

    static std::vector<block_info> get_block_info_from_config(const std::string &filename) {
        auto blkFilename = config::get_block_path() + "/" + filename;
        std::ifstream in(blkFilename, std::ios::binary);
        if(!in.is_open()) {
            spdlog::error("[exchange::blocks] 板块文件[{}], 打开失败", blkFilename);
            return {};
        }
        std::vector<block_info> list;
        std::string tmp_line;
        while (std::getline(in, tmp_line)) {
            try {
                std::string line = charsets::gbk_to_utf8(tmp_line);
                auto arr = strings::split(line, '|');
                if(arr.size()>=6) {
                    block_info bi={};
                    bi.name = arr[0];
                    bi.code = arr[1];
                    bi.type = std::stoi(arr[2]);
                    bi.Block = arr[5];
                    list.emplace_back(bi);
                }
            } catch(...) {
                continue;
            }
        }
        in.close();
        return list;
    }

    std::vector<block_info> load_index_block_infos() {
        std::vector<block_info> bis;
        auto bks = {"tdxzs.cfg", "tdxzs3.cfg"};
        std::unordered_map<std::string, block_info> tmp_map{};
        for (auto const & name : bks) {
            auto list = get_block_info_from_config(name);
            if (list.empty()) {
                continue;
            }
            //bis.insert(bis.end(), list.begin(), list.end());
            for(auto const &v : list) {
                auto it = tmp_map.find(v.code);
                if(it == tmp_map.end()) {
                    bis.emplace_back(v);
                    tmp_map[v.code] = v;
                }
            }
        }
        return bis;
    }

    struct IndustryInfo {
        int MarketId;     // 市场代码
        std::string Code; // 股票代码
        std::string Block; // 行业板块代码
        std::string Block5; // 二级行业板块代码
        std::string XBlock; // x行业代码
        std::string XBlock5; // x二级行业代码

        friend std::ostream &operator<<(std::ostream &os, const IndustryInfo &info) {
            os << "MarketId: " << info.MarketId << " Code: " << info.Code << " Block: " << info.Block << " Block5: "
               << info.Block5 << " XBlock: " << info.XBlock << " XBlock5: " << info.XBlock5;
            return os;
        }
    };

    static std::vector<IndustryInfo> load_industry_blocks() {
        std::string hyfile = "tdxhy.cfg";
        std::string cacheFilename = config::get_block_path() + "/" + hyfile;

        std::ifstream file(cacheFilename, std::ios::binary);
        if (!file.is_open()) {
            spdlog::error("[exchange::blocks] 板块文件[{}], 打开失败", cacheFilename);
            return {};
        }

        std::vector<IndustryInfo> hys;
        std::string line;

        while (std::getline(file, line)) {
            line = charsets::gbk_to_utf8(line); // GBK转UTF-8
            std::vector<std::string> arr = strings::split(line, '|');

            if (arr.size() < 3) {
                continue;
            }

            const std::string& bc = arr[2];
            std::string bc5 = bc;
            if (bc5.length() >= 5) {
                bc5 = bc5.substr(0, 5);
            }

            std::string xbc, xbc5;
            if (arr.size() >= 6) {
                xbc5 = arr[5];
                if (xbc5.length() >= 6) {
                    xbc = xbc5.substr(0, 5);
                }
            }
            auto marketId = std::stoi(arr[0]);
            if (marketId == exchange::BeiJing) {
                continue;
            }

            IndustryInfo hy={};
            hy.MarketId = marketId;
            hy.Code = exchange::CorrectSecurityCode(arr[1]);
            hy.Block = bc;
            hy.Block5 = bc5;
            hy.XBlock = xbc;
            hy.XBlock5 = xbc5;

            hys.push_back(hy);
        }

        file.close();
        return hys;
    }

    std::vector<std::string> industry_constituent_stock_list(const std::vector<IndustryInfo>& hys, const std::string& block) {
        std::vector<std::string> list;
        for (const auto& v : hys) {
            if (v.Block5.starts_with(block) || v.XBlock5.starts_with(block)) {
                list.push_back(v.Code);
            } else if (v.Block5 == block || v.Block == block || v.XBlock5 == block || v.XBlock == block) {
                list.push_back(v.Code);
            }
        }
        if (!list.empty()) {
            std::sort(list.begin(), list.end());
        }
        return list;
    }

    static std::vector<block_info> parse_and_generate_block_file() {
        auto blockInfos = load_index_block_infos();
        tsl::robin_map<std::string, std::string> block2Name{};
        for(auto const & v : blockInfos) {
            block2Name[v.Block] = v.name;
            //spdlog::debug("{} -> {}", v.Block, v.name);
        }
        auto bks = {level1::BLOCK_DEFAULT, level1::BLOCK_GAINIAN, level1::BLOCK_FENGGE, level1::BLOCK_ZHISHU};
        tsl::robin_map<std::string, block_info> name2block{};
        for(auto const &filename : bks) {
            auto bi = parse_block_raw_data(filename);
            if (bi.empty()) {
                continue;
            }
            for(auto const &bk : bi) {
                auto blockName = bk.name;
                auto it = block2Name.find(blockName);
                if (it != block2Name.end()) {
                    blockName = it->second;
                }
                name2block[blockName] = bk;
            }
        }

        // 行业板块数据
        auto hys = load_industry_blocks();
        for(auto & blockInfo : blockInfos) {
            auto v = &blockInfo;
            v->code = exchange::CorrectSecurityCode(v->code);
            auto bn = v->name;
            auto it = name2block.find(bn);
            if (it != name2block.end()) {
                auto _info = it->second;
                std::vector<std::string> list{};
                for (auto const &symbol : _info.ConstituentStocks) {
                    if (symbol.length() < 5) {
                        continue;
                    }
                    auto [marketId, prefix, x2] = exchange::DetectMarket(symbol);
                    if (marketId == exchange::BeiJing) {
                        continue;
                    }
                    list.emplace_back(prefix+symbol);
                }
                blockInfo.num = int(_info.num);
                blockInfo.ConstituentStocks = list;
                continue;
            }
            auto &bc        = v->Block;
            auto stockList = industry_constituent_stock_list(hys, bc);
            if (!stockList.empty()) {
                blockInfo.num = u16(stockList.size());
                blockInfo.ConstituentStocks = stockList;
            }
        }
        blockInfos.erase(std::remove_if(blockInfos.begin(),
                                        blockInfos.end(),
                                        [](const block_info &bi) {return bi.ConstituentStocks.empty();}),
                         blockInfos.end());
        return blockInfos;
    }

    static auto global_sector_once = RollingOnce::create("exchange-sector", cron_expr_daily_9am);
    static std::vector<block_info> global_sector_list;
    static tsl::robin_map<std::string, block_info> global_sector_map;

    // 同步板块数据
    std::vector<block_info> sync_block_files() {
        spdlog::debug("[exchange::blocks] {}", __FUNCTION__);

        // 检查本地的配置文件, 如果不存在, 则从内部导出
        for(auto const &rc : resources_meta_block_files) {
            auto cfgFilename = config::get_block_path() + "/" + rc.filename;
            if(!fs::exists(cfgFilename) || fs::file_size(cfgFilename) < 10) {
                const char *embed_data = reinterpret_cast<const char *>(rc.data.data());
                size_t embed_size = rc.data.size();
                io::write_file(cfgFilename, embed_data, embed_size);
            }
        }

        auto bks = {level1::BLOCK_DEFAULT, level1::BLOCK_GAINIAN, level1::BLOCK_FENGGE, level1::BLOCK_ZHISHU};
        for(auto const &filename : bks) {
            download_block_raw_data(filename);
        }
        //updateCacheBlockFile();
        global_sector_list = parse_and_generate_block_file();
        if(!global_sector_list.empty()) {
            global_sector_map.clear();
            for(auto const &v : global_sector_list) {
                global_sector_map.insert({v.code, v});
            }
        }
        return global_sector_list;
    }

    // 如果调用频繁耗时是比较大
    std::vector<block_info> get_sector_list() {
        global_sector_once->Do(sync_block_files);
        return global_sector_list;
    }

    // 如果调用频繁耗时是比较大
    tsl::robin_map<std::string, block_info> get_sector_map() {
        global_sector_once->Do(sync_block_files);
        return global_sector_map;
    }

    std::optional<block_info> get_sector_info(const std::string &code) {
        global_sector_once->Do(sync_block_files);
        auto securityCode = exchange::CorrectSecurityCode(code);
        //auto map = get_sector_map();
        auto it = global_sector_map.find(securityCode);
        if (it != global_sector_map.end()) {
            return it->second; // 返回指针
        } else {
            return std::nullopt;      // 未找到返回空指针
        }
    }


} // namespace exchange

