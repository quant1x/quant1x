#include <quant1x/contrib/data/tdx/sector.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <tsl/robin_map.h>

#include <spdlog/spdlog.h>
#include <minizip/unzip.h>

#include <quant1x/config/base.h>
#include <quant1x/data/market.h>
#include <quant1x/io/csv-writer.h>
#include <quant1x/data/meta/calendar.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/data/status.h>
#include <quant1x/encoding/charsets.h>
#include <quant1x/runtime/once.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/contrib/data/tdx/level1/std/block.h>


namespace quant1x::contrib::data::tdx::sector {

    // =========================================================================
    //  匿名命名空间: 内部辅助类型
    // =========================================================================
    namespace {

        class MiniZipExtractor {
        public:
            // allowedFiles: if empty, extract all files; otherwise only files present in this list
            bool extract(const std::string& zipPath, const std::string& outputDir,
                         const std::vector<std::string>& allowedFiles = {}) {
                // 保存允许的文件名集合（支持完整条目名或仅文件名）
                allowed_files_.clear();
                for (auto const &f : allowedFiles) {
                    allowed_files_.insert(f);
                }

                // 打开ZIP文件
                unzFile zipfile = unzOpen(zipPath.c_str());
                if (!zipfile) {
                    spdlog::error("[tdx::sector] 无法打开ZIP文件: {}", zipPath);
                    return false;
                }

                // 创建输出目录（检查错误）
                {
                    auto ec = filesystem::mkdirs(outputDir, true);
                    if (ec) {
                        spdlog::error("[tdx::sector] 无法创建输出目录[{}]: {}", outputDir, ec.message());
                        unzClose(zipfile);
                        return false;
                    }
                }

                // 获取ZIP文件信息
                unz_global_info global_info;
                if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK) {
                    spdlog::error("[tdx::sector] 无法读取ZIP文件信息: {}", zipPath);
                    unzClose(zipfile);
                    return false;
                }

                // 遍历所有文件
                if (unzGoToFirstFile(zipfile) == UNZ_OK) {
                    do {
                        extract_currentFile(zipfile, outputDir);
                    } while (unzGoToNextFile(zipfile) == UNZ_OK);
                }

                unzClose(zipfile);
                return true;
            }

        private:
            std::unordered_set<std::string> allowed_files_;

            void extract_currentFile(unzFile zipfile, const std::string& outputDir) {
                char filename[256];
                unz_file_info file_info;

                // 获取文件信息
                if (unzGetCurrentFileInfo(zipfile, &file_info, filename, 
                                        sizeof(filename), NULL, 0, NULL, 0) != UNZ_OK) {
                    return;
                }

                std::string fname(filename);
                std::string fullPath = outputDir + "/" + fname;

                // 如果指定了允许的文件列表，则只有在该列表中的文件才会被解压
                if (!allowed_files_.empty()) {
                    // 支持匹配完整条目名以及仅文件名
                    std::string baseName = std::filesystem::path(fname).filename().string();
                    if (allowed_files_.find(fname) == allowed_files_.end() && allowed_files_.find(baseName) == allowed_files_.end()) {
                        //std::cout << "跳过: " << filename << std::endl;
                        return;
                    }
                }

                // 检查是否是目录
                if (fname.size() > 0 && fname[fname.size() - 1] == '/') {
                    // 创建目录（检查错误）
                    {
                        auto ec = filesystem::mkdirs(fullPath, true);
                        if (ec) {
                            spdlog::error("[tdx::sector] 无法创建目录[{}]: {}", fullPath, ec.message());
                            return; // 跳过该条目
                        }
                    }
                } else {
                    // 创建父目录（检查错误）
                    if (!createParentDirectory(fullPath)) {
                        spdlog::error("[tdx::sector] 无法创建父目录，跳过文件: {}", fullPath);
                        return;
                    }
                    
                    // 打开ZIP中的文件
                    if (unzOpenCurrentFile(zipfile) == UNZ_OK) {
                        // 创建输出文件（使用 C++ 标准库）
                        std::ofstream outFile(fullPath, std::ios::binary);
                        if (outFile.is_open()) {
                            // 读取并写入文件
                            char buffer[8192];
                            int bytesRead;
                            while ((bytesRead = unzReadCurrentFile(zipfile, buffer, sizeof(buffer))) > 0) {
                                outFile.write(buffer, static_cast<std::streamsize>(bytesRead));
                            }
                            outFile.close();
                        }
                        unzCloseCurrentFile(zipfile);
                    }
                }
            }

            static bool createParentDirectory(const std::string& filepath) {
                size_t pos = filepath.find_last_of("/\\");
                if (pos != std::string::npos) {
                    std::string dir = filepath.substr(0, pos);
                    auto ec = filesystem::mkdirs(dir, true);
                    if (ec) {
                        spdlog::error("[tdx::sector] createParentDirectory failed for {}: {}", dir, ec.message());
                        return false;
                    }
                }
                return true;
            }
        };

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

        // =========================================================================
        //  板块数据文件常量
        // =========================================================================
        // 原始板块数据文件 (二进制格式, 需要从服务器下载后解析)
        constexpr const char* const BLOCK_DEFAULT = "block.dat";    // 默认板块 (早期)
        constexpr const char* const BLOCK_GAINIAN = "block_gn.dat"; // 概念板块
        constexpr const char* const BLOCK_FENGGE  = "block_fg.dat"; // 风格板块
        constexpr const char* const BLOCK_ZHISHU  = "block_zs.dat"; // 指数板块

        // 板块压缩包及行业配置文件
        constexpr const char* const BLK_ZIP_FILENAME       = "zhb.zip";   // 板块数据压缩包
        constexpr const char* const BLK_INDUSTRY_FILENAME  = "tdxhy.cfg"; // 行业板块配置

        // 需要从压缩包中解压的配置文件列表
        static const std::vector<std::string> NEED_BLK_FILES = {"tdxzs.cfg", "tdxzs3.cfg"};

    }  // namespace

    // =========================================================================
    //  operator<<
    // =========================================================================
    std::ostream &operator<<(std::ostream &os, const quant1x::data::schema::Sector &info) {
        os << "{code:" << info.code
           << ", name:" << info.name
           << ", type:" << info.type
           << ", count:" << info.count
           << ", constituent_stocks:[";
        bool first = true;
        for(auto const & v : info.constituent_stocks) {
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

    // =========================================================================
    //  底层数据操作: 下载 & 解析原始板块文件
    // =========================================================================

    /**
     * @brief 下载区块原始数据到本地文件
     *
     * 该函数负责从远程服务器下载指定区块的原始数据，并保存到本地配置的区块路径下。
     * 如果文件不存在、文件大小为0或需要更新时，会触发下载流程。
     *
     * @param filename 要下载的区块文件名（不包含路径）
     *
     * @note 下载过程采用分块传输方式，每次请求最大传输 BLOCK_CHUNKS_SIZE 大小的数据块
     * @note 函数内部会检查文件路径有效性，并自动创建必要的目录结构
     *
     * @throws 无显式抛出异常，但可能因以下原因记录错误日志：
     *         - 文件打开失败
     *         - 网络通信错误（通过level1::process隐式处理）
     *
     * @warning 函数会清空已存在的目标文件内容（使用ios::trunc模式）
     */
    void download_block_raw_data(const std::string &filename) {
        auto blkFilename = config::get_meta_path() + "/" + filename;
        bool create_or_update  = quant1x::data::should_initialize_file(blkFilename);
        if(create_or_update) {
            auto ec = filesystem::check_filepath(blkFilename, true);
            ec.clear();
            std::ofstream file(blkFilename, std::ios::binary|std::ios::out|std::ios::trunc);
            if(!file.is_open()) {
                spdlog::error("[tdx::sector] Failed to open file: {}", filename);
                return;
            }
            spdlog::debug("[tdx::sector] open file: {}", filename);
            auto conn_ptr = get_std_conn();
            if (!conn_ptr) {
                spdlog::error("sector: get_std_conn failed");
                return;
            }
            auto &socket = conn_ptr->socket();
            for(u32 start = 0;;) {
                BlockFileContext msg(filename, start);
                auto             err = transact_message_sync(socket, msg);
                if (err.value() != 0) {
                    spdlog::error("sector: process BlockFileContext for {} at offset {} failed: {}", filename, start, err.message());
                    return;
                }
                auto data = msg.Data;
                if( msg.DataSize > 0) {
                    file.write(reinterpret_cast<const char *>(data.data()), msg.DataSize);
                }
                if(msg.DataSize < quant1x::contrib::data::tdx::BLOCK_CHUNKS_SIZE) {
                    break;
                }
                start+= msg.DataSize;
            }
            file.close();
            spdlog::debug("[tdx::sector] close file: {}", filename);
        }
    }

    /**
     * @brief 解析板块原始数据文件，提取板块信息及其成分股列表
     *
     * @param filename 板块数据文件名（不包含路径）
     * @return std::vector<quant1x::data::schema::Sector> 解析得到的板块信息列表，包含板块名称、类型及成分股代码
     * @throws 无显式抛出异常，但会通过spdlog记录文件打开失败错误
     *
     * @note 文件路径由config::get_meta_path()确定，文件格式为二进制
     * @note 文件前384字节为头部信息，需要跳过
     * @note 板块名称使用GBK编码，内部会转换为UTF-8
     */
    std::vector<quant1x::data::schema::Sector> parse_block_raw_data(const std::string &filename) {
        auto          blkFilename = config::get_meta_path() + "/" + filename;
        std::ifstream in(blkFilename, std::ios::binary);
        if(!in.is_open()) {
            spdlog::error("[tdx::sector] 板块文件[{}], 打开失败", blkFilename);
            return {};
        }
        std::vector<uint8_t> buf((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        BinaryStream bs(buf);
        bs.skip(384);
        u16 Count = bs.get_u16();
        std::vector<quant1x::data::schema::Sector> list;
        for (int i = 0; i < Count; i++) {
            quant1x::data::schema::Sector bi{};
            u8 tmpBuf1[2813] = {0};
            bs.get_array(tmpBuf1);
            BinaryStream bs1(tmpBuf1);
            std::string tmp = bs1.get_string(9);
            bi.name = charsets::gbk_to_utf8(tmp);
            bi.count = bs1.get_u16();
            bi.type = bs1.get_u16();
            u8 tmpBuf2[400*7];
            bs1.get_array(tmpBuf2);
            BinaryStream bs2(tmpBuf2);
            bi.constituent_stocks.resize(bi.count); // 成分股
            for (int j = 0; j < bi.count; j++) {
                std::string symbol = bs2.get_string(7);
                bi.constituent_stocks[j] = symbol;
            }
            list.emplace_back(bi);
        }
        in.close();
        return list;
    }

    // =========================================================================
    //  配置类板块数据加载
    // =========================================================================

    /**
     * @brief 从配置文件读取板块信息
     *
     * 解析指定板块配置文件，将每行数据转换为BlockIndexEntry结构体
     *
     * @param filename 板块配置文件名（不含路径）
     * @return std::vector<quant1x::data::schema::Sector> 解析成功的板块信息列表，失败返回空vector
     * @throws 无显式抛出异常，但内部可能因编码转换或数字解析失败跳过错误行
     * @note 文件路径通过config::get_meta_path()获取，编码格式为GBK需转UTF-8
     */
    static std::vector<quant1x::data::schema::Sector> get_block_info_from_config(const std::string &filename) {
        auto          blkFilename = config::get_meta_path() + "/" + filename;
        std::ifstream in(blkFilename, std::ios::binary);
        if(!in.is_open()) {
            spdlog::error("[tdx::sector] 板块文件[{}], 打开失败", blkFilename);
            return {};
        }
        std::vector<quant1x::data::schema::Sector> list;
        std::string tmp_line;
        while (std::getline(in, tmp_line)) {
            try {
                std::string line = charsets::gbk_to_utf8(tmp_line);
                auto arr = strings::split(line, '|');
                if(arr.size()>=6) {
                    quant1x::data::schema::Sector bi={};
                    bi.name = arr[0];
                    bi.code = arr[1];
                    bi.type = std::stoi(arr[2]);
                    bi.block = arr[5];
                    list.emplace_back(bi);
                }
            } catch(...) {
                continue;
            }
        }
        in.close();
        return list;
    }

    /**
     * @brief 加载并合并所有需要的区块信息
     *
     * 从配置文件中读取所有需要的区块信息，并合并重复的区块代码，
     * 确保返回的区块信息列表中每个区块代码唯一。
     *
     * @return std::vector<quant1x::data::schema::Sector> 合并后的区块信息列表，每个区块代码只出现一次
     * @note 内部使用临时哈希表来检测和过滤重复的区块代码
     */
    std::vector<quant1x::data::schema::Sector> load_index_block_infos() {
        std::vector<quant1x::data::schema::Sector>                     bis;
        auto                                        bks = NEED_BLK_FILES;
        std::unordered_map<std::string, quant1x::data::schema::Sector> tmp_map{};
        for (auto const & name : bks) {
            auto list = get_block_info_from_config(name);
            if (list.empty()) {
                continue;
            }
            
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

    // =========================================================================
    //  行业板块数据加载与查询
    // =========================================================================

    /**
     * @brief 加载行业板块信息
     *
     * 从配置的板块文件中读取行业板块数据，解析并转换为IndustryInfo结构体列表。
     * 文件格式为GBK编码，每行以'|'分隔，包含市场ID、证券代码、板块名称等信息。
     *
     * @return std::vector<IndustryInfo> 解析后的行业板块信息列表，若文件打开失败则返回空列表
     * @throws 无显式抛出异常，但可能因文件操作或字符串转换产生隐式异常
     *
     * @note 1. 自动跳过北京市场的板块数据
     *       2. 板块名称超过5字符时截断为前5字符
     *       3. 证券代码会经过CorrectSecurityCode校正
     */
    static std::vector<IndustryInfo> load_industry_blocks() {
        std::string hyfile        = BLK_INDUSTRY_FILENAME;
        std::string cacheFilename = config::get_meta_path() + "/" + hyfile;

        std::ifstream file(cacheFilename, std::ios::binary);
        if (!file.is_open()) {
            spdlog::error("[tdx::sector] 板块文件[{}], 打开失败", cacheFilename);
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
            // if (marketId == static_cast<int>(exchange::ExchangeId::BeiJing)) {
            //     continue;
            // }

            IndustryInfo hy={};
            hy.MarketId  = marketId;
            hy.Code = arr[1];
            hy.Block = bc;
            hy.Block5 = bc5;
            hy.XBlock = xbc;
            hy.XBlock5 = xbc5;

            hys.emplace_back(hy);
        }

        file.close();
        return hys;
    }

    /**
     * @brief 根据行业板块名称获取该板块下的所有股票代码列表
     *
     * @param hys 行业信息列表，包含各股票的板块分类信息
     * @param block 要查询的板块名称
     * @return std::vector<std::string> 该板块下的所有股票代码列表，按字母顺序排序
     *
     * @note 匹配规则：检查股票的Block5、XBlock5、Block和XBlock字段，
     *       如果任一字段以指定板块名称开头或完全匹配，则包含该股票代码
     */
    std::vector<std::string> industry_constituent_stock_list(const std::vector<IndustryInfo> &hys,
                                                             const std::string               &block) {
        std::vector<std::string> list;
        for (const auto &v : hys) {
            if (v.Block5.starts_with(block) || v.XBlock5.starts_with(block) ||
                v.Block5 == block || v.Block == block || v.XBlock5 == block || v.XBlock == block) {
                list.emplace_back(v.Code);
            }
        }
        if (!list.empty()) {
            std::sort(list.begin(), list.end());
        }
        return list;
    }

    // =========================================================================
    //  板块文件合并生成
    // =========================================================================

    /**
     * @brief 解析并生成板块信息文件
     *
     * 该函数执行以下操作：
     * 1. 加载基础板块信息
     * 2. 解析预定义的板块数据文件(默认板块、概念板块、风格板块、指数板块)
     * 3. 加载行业板块数据
     * 4. 合并板块信息并修正证券代码
     * 5. 过滤掉无成分股的板块
     *
     * @return std::vector<quant1x::data::schema::Sector> 返回处理后的板块信息列表，已过滤掉无成分股的板块
     * @note 函数内部会跳过北京市场的证券代码
     */
    static std::vector<quant1x::data::schema::Sector> parse_and_generate_block_file() {
        auto                                     blockInfos = load_index_block_infos();
        tsl::robin_map<std::string, std::string> block2Name{};
        for(auto const & v : blockInfos) {
            block2Name[v.block] = v.name;
            spdlog::debug("[parse_and_generate_block_file/block2Name] {} -> code={}, name={}", v.block, v.code, v.name);
        }
        auto bks = {
            BLOCK_DEFAULT,
            BLOCK_GAINIAN,
            BLOCK_FENGGE,
            BLOCK_ZHISHU
        };
        tsl::robin_map<std::string, quant1x::data::schema::Sector> name2block{};
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
            v->code = quant1x::data::correct_security_code(v->code);
            auto bn = v->name;
            auto it = name2block.find(bn);
            bool is_target = (v->code == "sh880915" || bn == "昨日突涨");
            if (it != name2block.end()) {
                auto _info = it->second;
                std::vector<std::string> list{};
                for (auto const &symbol : _info.constituent_stocks) {
                    if (symbol.length() < 5) {
                        continue;
                    }
                    auto inst = quant1x::data::detect_symbol(symbol);
                    // auto [marketId, prefix, x2] = exchange::DetectMarket(symbol);
                    // if (marketId == exchange::ExchangeId::BeiJing) {
                    //     continue;
                    // }
                    list.emplace_back(inst.symbol());
                }
                blockInfo.count = int(_info.count);
                blockInfo.constituent_stocks = list;
                if (is_target) {
                    spdlog::debug("[sector] 昨日突涨: found in name2block, stocks count={}", list.size());
                }
                continue;
            }
            auto &bc        = v->block;
            if (is_target) {
                spdlog::debug("[sector] 昨日突涨: NOT in name2block, block field='{}', checking industry...", bc);
            }
            auto rawList = industry_constituent_stock_list(hys, bc);
            if (!rawList.empty()) {
                std::vector<std::string> stockList;
                for (auto const &s : rawList) {
                    auto inst = quant1x::data::detect_symbol(s);
                    stockList.emplace_back(inst.symbol());
                }
                blockInfo.count = u16(stockList.size());
                blockInfo.constituent_stocks = stockList;
                if (is_target) {
                    spdlog::debug("[sector] 昨日突涨: got {} stocks from industry, block='{}'", stockList.size(), bc);
                }
            } else {
                if (is_target) {
                    spdlog::debug("[sector] 昨日突涨: industry_constituent_stock_list returned EMPTY for block='{}'", bc);
                }
            }
        }
        blockInfos.erase(std::remove_if(blockInfos.begin(),
                                        blockInfos.end(),
                                        [](const quant1x::data::schema::Sector &bi) {
                                            if (bi.code == "sh880915") {
                                                spdlog::debug("[sector] 昨日突涨: ERASED because constituent_stocks is empty");
                                            }
                                            return bi.constituent_stocks.empty();
                                        }),
                         blockInfos.end());
        return blockInfos;
    }

    // =========================================================================
    //  全局状态
    // =========================================================================
    static auto                                                     global_sector_once = RollingOnce::create("exchange-sector", quant1x::config::GLOBAL_CRON_EXPR_DAILY_INIT);
    static std::vector<quant1x::data::schema::Sector>               global_sector_list;
    static tsl::robin_map<std::string, quant1x::data::schema::Sector> global_sector_map;

    // =========================================================================
    //  公开 API
    // =========================================================================

    // 同步板块数据
    std::vector<quant1x::data::schema::Sector> sync_block_files() {
        // 从服务器通过协议下载最新的板块文件
        auto bks = {
            BLK_INDUSTRY_FILENAME,
            BLK_ZIP_FILENAME,
            BLOCK_DEFAULT,
            BLOCK_GAINIAN,
            BLOCK_FENGGE,
            BLOCK_ZHISHU
        };
        for(auto const &filename : bks) {
            download_block_raw_data(filename);
        }
        MiniZipExtractor extractor;
        extractor.extract(config::get_meta_path() + "/" + BLK_ZIP_FILENAME, config::get_meta_path(), NEED_BLK_FILES);
        //updateCacheBlockFile();
        global_sector_list = parse_and_generate_block_file();
        if(!global_sector_list.empty()) {
            global_sector_map.clear();
            for(auto const &v : global_sector_list) {
                global_sector_map.insert({v.code, v});
            }
        }

        // 将 global_sector_list 写入 CSV 文件，使用项目的 io::CSVWriter
        // 成分股以逗号分隔的字符串形式写入单列
        // 文件名使用最后一个交易日日期, 与 Python get_sector_filename() 对齐
        try {
            auto cache_date = quant1x::data::meta::last_trading_day(
                quant1x::data::meta::Timestamp::now()).only_date();
            auto csvPath = config::get_meta_path() + "/blocks." + cache_date;
            io::CSVWriter writer(csvPath);
            // CSV 头 (列顺序与 Python sector.py 对齐)
            writer.write_row("name", "code", "type", "count", "constituent_stocks");
            for (auto const &s : global_sector_list) {
                std::string stocks_str;
                for (size_t i = 0; i < s.constituent_stocks.size(); ++i) {
                    if (i > 0) stocks_str += ",";
                    stocks_str += s.constituent_stocks[i];
                }
                writer.write_row(s.name, s.code, s.type, s.count, stocks_str);
            }
            spdlog::debug("[tdx::sector] wrote global sector csv: {}", csvPath);
        } catch (const std::exception &e) {
            spdlog::error("[tdx::sector] exception when writing sector csv: {}", e.what());
        }

        return global_sector_list;
    }

    // 如果调用频繁耗时是比较大
    std::vector<quant1x::data::schema::Sector> get_sector_list() {
        global_sector_once->Do(sync_block_files);
        return global_sector_list;
    }

    // 如果调用频繁耗时是比较大
    tsl::robin_map<std::string, quant1x::data::schema::Sector> get_sector_map() {
        global_sector_once->Do(sync_block_files);
        return global_sector_map;
    }

    std::optional<quant1x::data::schema::Sector> get_sector_info(const std::string &symbol) {
        global_sector_once->Do(sync_block_files);
        auto inst = quant1x::data::detect_symbol(symbol);
        //auto map = get_sector_map();
        auto it = global_sector_map.find(inst.symbol());
        if (it != global_sector_map.end()) {
            return it->second; // 返回指针
        } else {
            return std::nullopt;      // 未找到返回空指针
        }
    }
} // namespace exchange
