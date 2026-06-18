#include <quant1x/contrib/data/tdx/sector.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <optional>
#include <sstream>
#include <tsl/robin_map.h>

#include <spdlog/spdlog.h>

#include <quant1x/config/base.h>
#include <quant1x/data/market.h>
#include <quant1x/data/meta/calendar.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/data/status.h>
#include <quant1x/encoding/charsets.h>
#include <quant1x/runtime/once.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/contrib/data/tdx/level1/std/block_info.h>
#include <quant1x/contrib/data/tdx/level1/std/block_meta.h>

namespace config = ::config;
using quant1x::contrib::data::tdx::BLOCK_CHUNKS_SIZE;
using quant1x::contrib::data::tdx::BLOCK_DEFAULT;
using quant1x::contrib::data::tdx::BLOCK_FENGGE;
using quant1x::contrib::data::tdx::BLOCK_GAINIAN;
using quant1x::contrib::data::tdx::BLOCK_ZHISHU;
using quant1x::contrib::data::tdx::BlockInfoMsg;

namespace quant1x::contrib::data::tdx::sector {

    namespace config = ::config;
    namespace data = quant1x::data;
    namespace meta = quant1x::data::meta;

    // ============================================================
    // 内部类型
    // ============================================================

    using BlockIndexEntry = std::tuple<std::string, std::string, int, std::string>; // name, code, type, block

    struct RawBlockRecord {
        std::string              block_name;
        uint16_t                 num = 0;
        uint16_t                 block_type = 0;
        std::vector<std::string> codes;
    };

    struct IndustryInfo {
        int         market_id = 0;
        std::string code;
        std::string block;
        std::string block5;
        std::string xblock;
        std::string xblock5;
    };

    // ============================================================
    // 缓存: 通过 RollingOnce 保证每日首次调用时初始化一次
    // ============================================================

    static auto sector_once = RollingOnce::create("tdx-sector", meta::cron_expr_daily_9am);
    static std::vector<meta::schema::Sector> g_cached_sectors;
    static tsl::robin_map<std::string, meta::schema::Sector> g_cached_sector_map;

    // ============================================================
    // 工具函数
    // ============================================================

    /// 从以 \0 结尾的 GBK 字节中提取字符串
    static std::string extract_null_terminated_gbk(const uint8_t *data, size_t len) {
        size_t end = 0;
        while (end < len && data[end] != 0) ++end;
        if (end == 0) return {};
        return charsets::gbk_to_utf8(std::string(reinterpret_cast<const char *>(data), end));
    }

    /// 从以 \0 结尾的 ASCII 字节中提取字符串
    static std::string extract_null_terminated_ascii(const uint8_t *data, size_t len) {
        size_t end = 0;
        while (end < len && data[end] != 0) ++end;
        return std::string(reinterpret_cast<const char *>(data), end);
    }

    /// 解析 JSON 字符串数组, 如 ["000001","600000"]
    static std::vector<std::string> parse_json_string_array(const std::string &json_str) {
        std::vector<std::string> result;
        if (json_str.empty() || json_str == "[]") return result;
        size_t pos = 0;
        while ((pos = json_str.find('"', pos)) != std::string::npos) {
            size_t end = json_str.find('"', pos + 1);
            if (end == std::string::npos) break;
            result.push_back(json_str.substr(pos + 1, end - pos - 1));
            pos = end + 1;
        }
        return result;
    }

    /// 生成 JSON 字符串数组
    static std::string to_json_string_array(const std::vector<std::string> &items) {
        if (items.empty()) return "[]";
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) oss << ",";
            oss << "\"" << items[i] << "\"";
        }
        oss << "]";
        return oss.str();
    }

    /// 解析 CSV 行 (处理双引号包裹字段)
    static std::vector<std::string> parse_csv_line(const std::string &line) {
        std::vector<std::string> fields;
        size_t pos = 0;
        while (pos < line.length()) {
            if (line[pos] == '"') {
                // 引号包裹的字段
                size_t end_quote = pos + 1;
                while (end_quote < line.length()) {
                    if (line[end_quote] == '"') {
                        if (end_quote + 1 < line.length() && line[end_quote + 1] == '"') {
                            end_quote += 2; // 转义引号
                        } else {
                            break;
                        }
                    } else {
                        ++end_quote;
                    }
                }
                std::string field = line.substr(pos + 1, end_quote - pos - 1);
                // 还原转义引号
                size_t dq = 0;
                while ((dq = field.find("\"\"", dq)) != std::string::npos) {
                    field.replace(dq, 2, "\"");
                    ++dq;
                }
                fields.push_back(field);
                pos = end_quote + 1;
                if (pos < line.length() && line[pos] == ',') ++pos;
            } else {
                // 非引号字段
                size_t comma = line.find(',', pos);
                if (comma == std::string::npos) {
                    fields.push_back(line.substr(pos));
                    break;
                }
                fields.push_back(line.substr(pos, comma - pos));
                pos = comma + 1;
            }
        }
        return fields;
    }

    // ============================================================
    // SectorType 名称映射
    // ============================================================

    std::string sector_type_name_by_code(int sector_code) {
        switch (static_cast<SectorType>(sector_code)) {
            case HANGYE:  return "行业";
            case DIQU:    return "地区";
            case GAINIAN: return "概念";
            case FENGGE:  return "风格";
            case ZHISHU:  return "指数";
            case YJHY:    return "研究行业";
            default:      return "未知";
        }
    }

    // ============================================================
    // 板块缓存文件路径
    // ============================================================

    std::string get_sector_filename() {
        auto now = meta::Timestamp::now();
        auto trade_day = meta::last_trading_day(now);
        return config::get_meta_path() + "/blocks." + trade_day.only_date();
    }

    // ============================================================
    // 从 level1 下载原始板块文件
    // ============================================================

    static std::optional<std::vector<uint8_t>> get_block_info_from_level1(const std::string &filename) {
        auto conn_ptr = get_std_conn();
        if (!conn_ptr) {
            spdlog::error("sector: get_std_conn failed");
            return std::nullopt;
        }
        auto &socket = conn_ptr->socket();

        uint32_t start = 0;
        std::vector<uint8_t> result;
        while (true) {
            BlockInfoMsg msg(filename, start);
            auto err = process_message(socket, msg);
            if (err.value() != 0) {
                spdlog::error("sector: process BlockInfoMsg for {} at offset {} failed: {}", filename, start, err.message());
                return std::nullopt;
            }
            if (msg.DataSize == 0) {
                return std::nullopt;
            }
            if (msg.DataSize > 0) {
                result.insert(result.end(), msg.Data.begin(), msg.Data.end());
            }
            if (msg.DataSize < BLOCK_CHUNKS_SIZE) {
                break;
            }
            start += msg.DataSize;
        }
        return result;
    }

    static std::optional<std::string> download_block_raw_data(const std::string &filename) {
        auto meta_path = config::get_meta_path();
        std::string filepath = meta_path + "/" + filename;

        // 文件已存在且不需要更新, 跳过
        {
            std::ifstream check(filepath);
            if (check.good()) {
                if (!data::should_initialize_file(filepath)) {
                    spdlog::debug("sector: {} exists and is up-to-date, skip download", filename);
                    return filepath;
                }
            }
        }

        auto data = get_block_info_from_level1(filename);
        if (!data.has_value() || data->empty()) {
            spdlog::warn("sector: failed to download {}", filename);
            return std::nullopt;
        }

        std::ofstream out(filepath, std::ios::binary);
        if (!out) {
            spdlog::error("sector: failed to open {} for writing", filepath);
            return std::nullopt;
        }
        out.write(reinterpret_cast<const char *>(data->data()), data->size());
        return filepath;
    }

    // ============================================================
    // 解析原始板块二进制文件
    // ============================================================

    static std::vector<RawBlockRecord> parse_raw_block_file(const std::string &block_filename) {
        auto meta_path = config::get_meta_path();
        std::string filepath = meta_path + "/" + block_filename;

        std::ifstream file(filepath, std::ios::binary);
        if (!file) return {};

        // 获取文件大小
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        // skip 384 bytes header
        if (file_size < 386) return {};
        file.seekg(384);

        // 读取 count (2 bytes little-endian)
        uint8_t cnt_buf[2];
        file.read(reinterpret_cast<char *>(cnt_buf), 2);
        uint16_t count = cnt_buf[0] | (cnt_buf[1] << 8);

        std::vector<RawBlockRecord> records;
        records.reserve(count);

        for (uint16_t i = 0; i < count; ++i) {
            std::vector<uint8_t> rec(2813);
            file.read(reinterpret_cast<char *>(rec.data()), 2813);
            if (file.gcount() < 2813) break;

            RawBlockRecord r;
            r.block_name = extract_null_terminated_gbk(rec.data(), 9);
            r.num        = rec[9] | (rec[10] << 8);
            r.block_type = rec[11] | (rec[12] << 8);

            // 400 个代码, 每个 7 字节, 从 offset 13 开始
            for (int ci = 0; ci < 400; ++ci) {
                size_t code_offset = 13 + ci * 7;
                if (code_offset + 7 > rec.size()) break;
                std::string code = extract_null_terminated_ascii(rec.data() + code_offset, 7);
                if (!code.empty()) {
                    r.codes.push_back(code);
                }
            }
            records.push_back(std::move(r));
        }
        return records;
    }

    // ============================================================
    // 解析配置文件 (tdxzs.cfg / tdxzs3.cfg)
    // ============================================================

    static std::vector<BlockIndexEntry> get_block_info_from_config(const std::string &cfg_name) {
        auto meta_path = config::get_meta_path();
        std::string filepath = meta_path + "/" + cfg_name;

        std::ifstream file(filepath);
        if (!file) {
            // 尝试 GBK 编码
            file.open(filepath, std::ios::binary);
            if (!file) return {};
            std::vector<char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
            auto content = charsets::gbk_to_utf8(std::string(bytes.data(), bytes.size()));
            std::istringstream ss(content);

            std::vector<BlockIndexEntry> entries;
            std::string line;
            while (std::getline(ss, line)) {
                if (line.empty()) continue;
                // 去掉 \r
                if (!line.empty() && line.back() == '\r') line.pop_back();
                if (line.empty()) continue;

                auto parts = parse_csv_line(line); // 用 | 分隔, 不过 CSV 解析器用逗号, 需要特殊处理
                // 手动按 | 分割
                std::vector<std::string> arr;
                size_t pos = 0;
                while (pos <= line.length()) {
                    size_t next = line.find('|', pos);
                    if (next == std::string::npos) {
                        arr.push_back(line.substr(pos));
                        break;
                    }
                    arr.push_back(line.substr(pos, next - pos));
                    pos = next + 1;
                }

                if (arr.size() < 4) continue;
                std::string name  = arr[0];
                std::string code  = arr[1];
                int         btype = std::stoi(arr[2]);
                std::string block = arr.size() > 5 ? arr[5] : "";
                entries.emplace_back(name, code, btype, block);
            }
            return entries;
        }

        // UTF-8 读取
        std::vector<BlockIndexEntry> entries;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            // 按 | 分割
            std::vector<std::string> arr;
            size_t pos = 0;
            while (pos <= line.length()) {
                size_t next = line.find('|', pos);
                if (next == std::string::npos) {
                    arr.push_back(line.substr(pos));
                    break;
                }
                arr.push_back(line.substr(pos, next - pos));
                pos = next + 1;
            }

            if (arr.size() < 4) continue;
            std::string name  = arr[0];
            std::string code  = arr[1];
            int         btype = std::stoi(arr[2]);
            std::string block = arr.size() > 5 ? arr[5] : "";
            entries.emplace_back(name, code, btype, block);
        }
        return entries;
    }

    // ============================================================
    // 行业配置 (tdxhy.cfg)
    // ============================================================

    static std::vector<IndustryInfo> load_industry_blocks() {
        auto meta_path = config::get_meta_path();
        std::string filepath = meta_path + "/tdxhy.cfg";

        std::vector<IndustryInfo> out;

        // 尝试 UTF-8
        {
            std::ifstream file(filepath);
            if (file) {
                std::string line;
                while (std::getline(file, line)) {
                    if (line.empty()) continue;
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    if (line.empty()) continue;

                    std::vector<std::string> arr;
                    size_t pos = 0;
                    while (pos <= line.length()) {
                        size_t next = line.find('|', pos);
                        if (next == std::string::npos) {
                            arr.push_back(line.substr(pos));
                            break;
                        }
                        arr.push_back(line.substr(pos, next - pos));
                        pos = next + 1;
                    }

                    if (arr.size() < 3) continue;
                    IndustryInfo info;
                    info.market_id = std::stoi(arr[0]);
                    info.code      = arr[1];
                    info.block     = arr[2];
                    info.block5    = info.block.length() >= 5 ? info.block.substr(0, 5) : info.block;
                    info.xblock5   = arr.size() > 5 ? arr[5] : "";
                    info.xblock    = info.xblock5.length() >= 5 ? info.xblock5.substr(0, 5) : info.xblock5;
                    out.push_back(std::move(info));
                }
                return out;
            }
        }

        // 尝试 GBK 编码
        filepath = meta_path + "/tdxhy.cfg";
        std::ifstream file(filepath, std::ios::binary);
        if (!file) return out;
        std::vector<char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        auto content = charsets::gbk_to_utf8(std::string(bytes.data(), bytes.size()));
        std::istringstream ss(content);

        std::string line;
        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            std::vector<std::string> arr;
            size_t pos = 0;
            while (pos <= line.length()) {
                size_t next = line.find('|', pos);
                if (next == std::string::npos) {
                    arr.push_back(line.substr(pos));
                    break;
                }
                arr.push_back(line.substr(pos, next - pos));
                pos = next + 1;
            }

            if (arr.size() < 3) continue;
            IndustryInfo info;
            info.market_id = std::stoi(arr[0]);
            info.code      = arr[1];
            info.block     = arr[2];
            info.block5    = info.block.length() >= 5 ? info.block.substr(0, 5) : info.block;
            info.xblock5   = arr.size() > 5 ? arr[5] : "";
            info.xblock    = info.xblock5.length() >= 5 ? info.xblock5.substr(0, 5) : info.xblock5;
            out.push_back(std::move(info));
        }
        return out;
    }

    /// 行业成分股列表 (对齐 Python industry_constituent_stock_list)
    static std::vector<std::string> industry_constituent_stock_list(const std::vector<IndustryInfo> &hys, const std::string &block) {
        std::vector<std::string> lst;
        for (const auto &v : hys) {
            bool matched = v.block5.rfind(block, 0) == 0   // block5 starts_with block
                        || v.xblock5.rfind(block, 0) == 0  // xblock5 starts_with block
                        || v.block5 == block
                        || v.block == block
                        || v.xblock5 == block
                        || v.xblock == block;
            if (matched) {
                lst.push_back(v.code);
            }
        }
        std::sort(lst.begin(), lst.end());
        lst.erase(std::unique(lst.begin(), lst.end()), lst.end());
        return lst;
    }

    // ============================================================
    // 解析并生成板块 CSV 缓存文件
    // ============================================================

    static std::optional<std::string> parse_and_generate_block_file() {
        // 1) 加载 zs* 配置文件
        std::vector<const char *> bks_cfg = {"tdxzs.cfg", "tdxzs3.cfg"};
        std::vector<BlockIndexEntry> block_index;
        tsl::robin_map<std::string, BlockIndexEntry> tmp_map;
        for (const auto *cfg : bks_cfg) {
            auto bi = get_block_info_from_config(cfg);
            for (auto &v : bi) {
                auto code = std::get<1>(v);
                if (tmp_map.find(code) != tmp_map.end()) continue;
                tmp_map[code] = v;
                block_index.push_back(std::move(v));
            }
        }

        if (block_index.empty()) {
            spdlog::warn("sector: no block index entries found from config files");
            return std::nullopt;
        }

        // block -> name mapping
        tsl::robin_map<std::string, std::string> block2name;
        for (const auto &v : block_index) {
            const auto &block = std::get<3>(v);
            if (!block.empty()) {
                block2name[block] = std::get<0>(v);
            }
        }

        // 2) 解析原始板块文件
        std::vector<const char *> raw_files = {
            BLOCK_DEFAULT, BLOCK_GAINIAN, BLOCK_FENGGE, BLOCK_ZHISHU
        };
        tsl::robin_map<std::string, RawBlockRecord> name2block;
        for (const auto *f : raw_files) {
            auto recs = parse_raw_block_file(f);
            for (auto &bk : recs) {
                auto it = block2name.find(bk.block_name);
                std::string resolved = it != block2name.end() ? it->second : bk.block_name;
                name2block[resolved] = std::move(bk);
            }
        }

        // 3) code->hy mapping
        tsl::robin_map<std::string, std::string> code2hy;
        for (const auto &v : block_index) {
            const auto &name  = std::get<0>(v);
            const auto &block = std::get<3>(v);
            if (name != block) {
                code2hy[block] = name;
            }
        }

        // 4) industry blocks
        auto hys = load_industry_blocks();

        // 5) 组装最终板块条目
        using Row = std::tuple<std::string, std::string, int, int, std::string, std::vector<std::string>>;
        std::vector<Row> rows;

        for (const auto &v : block_index) {
            const auto &v_name = std::get<0>(v);
            auto it = name2block.find(v_name);
            if (it != name2block.end()) {
                auto &info = it->second;
                std::vector<std::string> entry_codes;
                for (const auto &sc : info.codes) {
                    if (sc.length() >= 5) {
                        entry_codes.push_back(sc);
                    }
                }
                std::sort(entry_codes.begin(), entry_codes.end());
                entry_codes.erase(std::unique(entry_codes.begin(), entry_codes.end()), entry_codes.end());

                rows.emplace_back(
                    v_name,
                    std::get<1>(v),
                    std::get<2>(v),
                    static_cast<int>(entry_codes.size()),
                    std::get<3>(v),
                    entry_codes
                );
                continue;
            }

            // fallback: industry mapping
            const auto &bc = std::get<3>(v);
            auto stock_list = industry_constituent_stock_list(hys, bc);
            if (!stock_list.empty()) {
                rows.emplace_back(
                    v_name,
                    std::get<1>(v),
                    std::get<2>(v),
                    static_cast<int>(stock_list.size()),
                    bc,
                    stock_list
                );
            }
        }

        // 过滤空条目
        rows.erase(std::remove_if(rows.begin(), rows.end(), [](const Row &r) { return std::get<5>(r).empty(); }), rows.end());

        if (rows.empty()) {
            spdlog::warn("sector: no valid block entries after assembly");
            return std::nullopt;
        }

        // 写入 CSV
        auto out_fn = get_sector_filename();
        // 确保目录存在
        auto parent = out_fn.substr(0, out_fn.find_last_of("/\\"));
        if (!parent.empty()) {
            // 创建目录 (简单方式: 尝试打开文件, 失败则警告)
            // 实际使用中 meta_path 应该已存在
        }

        std::ofstream csv(out_fn);
        if (!csv) {
            spdlog::error("sector: failed to open CSV for writing: {}", out_fn);
            return std::nullopt;
        }

        // CSV header
        csv << "name,code,type,count,block,constituent_stocks\n";
        for (const auto &row : rows) {
            const auto &name    = std::get<0>(row);
            const auto &code    = std::get<1>(row);
            int         btype   = std::get<2>(row);
            int         cnt     = std::get<3>(row);
            const auto &block   = std::get<4>(row);
            const auto &cs      = std::get<5>(row);
            auto cs_json = to_json_string_array(cs);

            csv << "\"" << name << "\","
                << "\"" << code << "\","
                << btype << ","
                << cnt << ","
                << "\"" << block << "\","
                << "\"" << cs_json << "\"\n";
        }
        csv.close();

        spdlog::info("sector: generated sector CSV: {} entries → {}", rows.size(), out_fn);
        return out_fn;
    }

    // ============================================================
    // 同步板块文件
    // ============================================================

    static void sync_block_files() {
        spdlog::info("sector: sync_block_files start");

        // 行业配置
        download_block_raw_data("tdxhy.cfg");

        // 下载 zhb.zip 并解压 (TODO: 需要 zip 库支持, 当前依赖外部工具预先生成 tdxzs.cfg / tdxzs3.cfg)
        // download_block_raw_data("zhb.zip");
        // if (zhb) { extract tdxzs.cfg and tdxzs3.cfg from zip }

        // 下载标准板块文件
        for (const auto *fname : {BLOCK_DEFAULT, BLOCK_GAINIAN, BLOCK_FENGGE, BLOCK_ZHISHU}) {
            download_block_raw_data(fname);
        }

        // 解析并生成 CSV
        auto result = parse_and_generate_block_file();
        if (result.has_value()) {
            spdlog::info("sector: sync_block_files done, output: {}", result.value());
        } else {
            spdlog::warn("sector: sync_block_files failed to generate CSV");
        }
    }

    // ============================================================
    // 加载缓存板块数据
    // ============================================================

    static void load_cache_block_infos() {
        auto bk_filename = get_sector_filename();

        // 如果 CSV 不存在或需要更新, 先同步
        bool need_sync = false;
        {
            std::ifstream check(bk_filename);
            if (!check.good()) {
                need_sync = true;
            }
        }
        if (!need_sync) {
            need_sync = data::should_initialize_file(bk_filename);
        }

        if (need_sync) {
            spdlog::info("sector: cache missing or outdated, triggering sync_block_files");
            sync_block_files();
        }

        // 从 CSV 加载
        std::ifstream csv(bk_filename);
        if (!csv) {
            spdlog::warn("sector: cannot open CSV cache: {}", bk_filename);
            return;
        }

        std::vector<meta::schema::Sector> sectors;
        tsl::robin_map<std::string, meta::schema::Sector> sector_map;
        std::string line;

        // 跳过 header
        std::getline(csv, line);

        while (std::getline(csv, line)) {
            if (line.empty()) continue;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            auto fields = parse_csv_line(line);
            if (fields.size() < 6) continue;

            meta::schema::Sector s;
            s.name               = fields[0];
            s.code               = fields[1];
            s.type               = std::stoi(fields[2]);
            s.count              = std::stoi(fields[3]);
            s.block              = fields[4];
            s.constituent_stocks = parse_json_string_array(fields[5]);

            sectors.push_back(s);
            sector_map[s.code] = s;
        }

        spdlog::info("sector: loaded {} sectors from cache", sectors.size());

        g_cached_sectors = std::move(sectors);
        g_cached_sector_map = std::move(sector_map);
    }

    // ============================================================
    // 公共 API
    // ============================================================

    std::vector<meta::schema::Sector> get_sector_list() {
        sector_once->Do([] {
            load_cache_block_infos();
        });
        return g_cached_sectors;
    }

    std::optional<meta::schema::Sector> get_sector_info(const std::string &symbol) {
        sector_once->Do([] {
            load_cache_block_infos();
        });

        auto inst = data::detect_symbol(symbol);
        auto it = g_cached_sector_map.find(inst.symbol());
        if (it != g_cached_sector_map.end()) {
            return it->second;
        }
        return std::nullopt;
    }

} // namespace quant1x::contrib::data::tdx::sector
