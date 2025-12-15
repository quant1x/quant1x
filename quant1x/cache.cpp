#include <quant1x/cache.h>
#include <quant1x/std/filepath.h>
#include <indicators/dynamic_progress.hpp>
#include <indicators/progress_bar.hpp>
#include <boost/pfr/core.hpp>
#include <csv2/writer.hpp>

namespace cache {

    namespace fs = std::filesystem;
    namespace mpb = indicators;

    namespace {
        // 常量定义
        constexpr const char *const timeLayoutOfState = "{:%H%M%S}";
        constexpr const char *const timeLayoutOfPhase = "{:%H:%M:%S}";
        const std::string lastUpdateTime = "22:00:00";
        const std::vector<std::string> allDateUpdateTimes = {"15:10:00", lastUpdateTime};
        const size_t default_concurrency_max = std::min<size_t>(std::thread::hardware_concurrency(), 8);
        const std::string default_concurrency_key = "default";
    } // 匿名命名空间

    std::string getVariablePath() {
        return config::default_cache_path() + "/var";
    }

    std::string stateFilename(const std::string& date, const exchange::timestamp& timestamp) {
        std::string fixedDate = exchange::timestamp(date).only_date();
        std::string tm = timestamp.toString(timeLayoutOfState).substr(0,6);

        std::string tmStr = fixedDate + "T" + tm;
        std::string filename = getVariablePath() + "/update." + tmStr;
        return filename;
    }


    bool checkUpdateState(const std::string& date, const exchange::timestamp& timestamp) {
        std::string filename = stateFilename(date, timestamp);
        return !fs::exists(filename);
    }

    void doneUpdate(const std::string& date, const exchange::timestamp& timestamp) {
        std::string filename = stateFilename(date, timestamp);
        auto err = filepath::check_filepath(filename, true);
        err.clear();
        io::write_file(filename);
    }

    bool cleanExpiredStateFiles() {
        std::string statePath = getVariablePath();
        std::string pattern = statePath + "/update.*";

        try {
            for (const auto& entry : fs::directory_iterator(statePath)) {
                if (entry.path().string().find("update.") != std::string::npos) {
                    fs::remove(entry.path());
                }
            }
            return true;
        } catch (const std::exception& e) {
            spdlog::error("Error cleaning state files: {}", e.what());
            return false;
        }
    }

    int update_with_adapters(const std::vector<cache::DataAdapter*> &adapters, const exchange::timestamp& feature_date) {
        auto const & config = config::global_config();
        auto const & cfg_concurrency = config.data.concurrency;
        // 隐藏终端光标以获得更流畅的显示效果
        mpb::show_console_cursor(false);

        // 创建多进度条管理器
        mpb::DynamicProgress<mpb::ProgressBar> bars;

        // 主进度条为适配器
        auto count = adapters.size();
        mpb::ProgressBar barMain{
            mpb::option::BarWidth{50},
            mpb::option::ForegroundColor{mpb::Color::cyan},
            mpb::option::Start{"["},
            mpb::option::Fill{"="},
            mpb::option::Lead{">"},
            mpb::option::Remainder{" "},
            mpb::option::End{"]"},
            mpb::option::ShowElapsedTime{true},
            mpb::option::ShowRemainingTime{true},
            mpb::option::FontStyles{std::vector<mpb::FontStyle>{mpb::FontStyle::bold}},
            mpb::option::ShowPercentage{true},
            mpb::option::ShowSpeed{true},
            mpb::option::MaxProgress{count}
        };
        bars.push_back(barMain);
        bars[0].set_progress(0);

        auto first = adapters[0]->Key();
        auto allCodes = instruments::GetCodeList();
        mpb::ProgressBar barCodes(
            mpb::option::BarWidth{50},
            mpb::option::ForegroundColor{mpb::Color::yellow},
            mpb::option::ShowElapsedTime{true},
            mpb::option::ShowRemainingTime{true},
            mpb::option::PrefixText{first + ": fetching..."},
            mpb::option::FontStyles{std::vector<mpb::FontStyle>{mpb::FontStyle::bold}},
            mpb::option::ShowPercentage{true},
            mpb::option::ShowSpeed{true},
            mpb::option::MaxProgress{allCodes.size()});
        bars.push_back(barCodes);

        // 缓存日期
        auto cache_date = exchange::next_trading_day(feature_date);

        // 线程池大小，根据CPU核心数调整
        //const size_t num_threads = std::min<size_t>(std::thread::hardware_concurrency(), 5);
        //const size_t num_cpus = getPhysicalCPUCount();
        // 默认并发数, 按照系统默认值预先设置
        size_t default_concurrency = default_concurrency_max;
        // 从配置文件读取默认的并发数, 如果存在则覆盖
        auto const & cfg_default = cfg_concurrency.find(default_concurrency_key);
        if (cfg_default != cfg_concurrency.end() && cfg_default->second > 0) {
            default_concurrency = cfg_default->second;
        }
        spdlog::info("[{}] concurrency={}", default_concurrency_key, default_concurrency);
        for (size_t idx = 0; idx < count; ++idx) {
            // 默认物理CPU数量的2倍与最大连接数一半, 取最小值
            //size_t concurrency = std::min(num_cpus*2, size_t(level1::_max_connections) / 2);

            auto* adapter = adapters[idx];
            std::string module_name = std::format("{}({}/{})", adapter->Key(), (idx+1), count);

            spdlog::info("[update] plugin={}, start", module_name);
            bars[0].set_option(mpb::option::PrefixText{module_name + ""});
            bars[1].set_option(mpb::option::PrefixText{module_name + ""});
            bars[1].set_progress(0);
            bars[1].mark_as_started();
            bars[1].set_option(mpb::option::Completed {false});
            auto codeCount = allCodes.size();
            bars[1].set_option(mpb::option::MaxProgress{codeCount+0});
            size_t num_threads = default_concurrency;
            auto const & cfg_adapter = cfg_concurrency.find(adapter->Key());
            if (cfg_adapter != cfg_concurrency.end() && cfg_adapter->second > 0) {
                num_threads = cfg_adapter->second;
            }
            spdlog::info("[{}] concurrency={}", adapter->Key(), num_threads);
            // 初始化特征适配器
            bool is_feature_adapter = false;
            std::string cache_filename;
            cache::FeatureAdapter *featureAdapter = nullptr;

            if((adapter->Kind() & cache::PluginMaskFeature) == cache::PluginMaskFeature) {
                featureAdapter = dynamic_cast<cache::FeatureAdapter*>(adapter);
                if(featureAdapter) {
                    //concurrency = std::min<size_t>(std::thread::hardware_concurrency(), 8);
                    featureAdapter->init(feature_date);
                    cache_filename = featureAdapter->Filename(cache_date);
                    is_feature_adapter = true;
                    spdlog::info("特征适配器[{}]初始化完成，缓存文件: {}", featureAdapter->Name(), cache_filename);
                }
            }

            // 线程安全数据结构
            struct ThreadResult {
                std::vector<std::pair<std::string, std::vector<std::string>>> data; // <code, values>
                std::mutex mutex;
            };
            std::vector<ThreadResult> thread_results(num_threads);
            std::atomic<size_t> processed_codes = 0;
            //std::mutex progress_mutex;

            // 线程处理函数
            auto process_batch = [&](/*const std::stop_token& stoken, */size_t thread_idx, size_t start, size_t end) {
                //(void) stoken;
                auto& result = thread_results[thread_idx];

                for (size_t i = start; i < end /*&& !stoken.stop_requested()*/; ++i) {
                    const auto& code = allCodes[i];
                    std::vector<std::string> values;

                    try {
                        // 调用适配器的方法
                        if(is_feature_adapter && featureAdapter) {
                            // 特征数据, 需要先clone一个实例, 然后用这个实例进行更新操作
                            auto feature = featureAdapter->clone();
                            feature->Update(code, feature_date);
                            values = feature->values();
                        } else {
                            // 基础数据是适配器自己内部聚合文件, 不需要外部干预
                            adapter->Update(code, feature_date);
                        }

                        // 线程安全地保存结果
                        if(is_feature_adapter && !values.empty()) {
                            std::lock_guard<std::mutex> lock(result.mutex);
                            result.data.emplace_back(code, std::move(values));
                        }

                        // 更新进度
                        size_t current = ++processed_codes;
                        {
                            //std::lock_guard<std::mutex> lock(progress_mutex);
                            std::string codePrefix = std::format("{}({}/{})", code, current, codeCount);
                            bars[1].set_option(mpb::option::PrefixText{codePrefix + ""});
                            bars[1].tick();
                        }
                    } catch (const std::exception &e) {
                        spdlog::error("处理代码 {} 时出错: {}", code, e.what());
                    } catch (...) {
                        spdlog::error("处理代码 {} 时发生未知错误", code);
                    }
                }
            };

            // 创建并分配线程任务
            std::vector<std::thread> workers;
            size_t batch_size = (allCodes.size() + num_threads - 1) / num_threads;

            for (size_t t = 0; t < num_threads; ++t) {
                size_t start = t * batch_size;
                size_t end = std::min(start + batch_size, allCodes.size());
                if (start < end) {
                    workers.emplace_back(process_batch, t, start, end);
                }
            }

            // 等待线程完成
            for (auto& worker : workers) {
                if (worker.joinable()) worker.join();
            }

            // 合并结果并保存特征数据
            if(is_feature_adapter && featureAdapter) {
                try {
                    // 1. 收集所有结果
                    std::vector<std::pair<std::string, std::vector<std::string>>> all_data;
                    for (auto& result : thread_results) {
                        std::lock_guard<std::mutex> lock(result.mutex);
                        all_data.insert(all_data.end(),
                                        std::make_move_iterator(result.data.begin()),
                                        std::make_move_iterator(result.data.end()));
                    }

                    // 2. 按原始代码顺序排序
                    std::unordered_map<std::string, size_t> code_order;
                    for (size_t i = 0; i < allCodes.size(); ++i) {
                        code_order[allCodes[i]] = i;
                    }

                    std::sort(all_data.begin(), all_data.end(),
                              [&code_order](const auto& a, const auto& b) {
                                  return code_order[a.first] < code_order[b.first];
                              });

                    // 3. 准备最终数据
                    std::vector<std::vector<std::string>> final_data;
                    final_data.emplace_back(featureAdapter->headers()); // 表头

                    for (const auto& item : all_data) {
                        if (!item.second.empty()) {
                            final_data.push_back(item.second);
                        }
                    }

                    // 4. 写入文件
                    if (!final_data.empty()) {
                        auto ec = filepath::check_filepath(cache_filename, true);
                        ec.clear();
                        std::ofstream out_file(cache_filename, std::ios::binary|std::ios::out | std::ios::trunc);
                        if (out_file) {
                            csv2::Writer<csv2::delimiter<','>> writer(out_file);
                            writer.write_rows(final_data);
                            out_file.close();

                            spdlog::info("成功写入 {} 行数据到 {}", final_data.size(), cache_filename);

                            // 验证文件
                            if (std::filesystem::exists(cache_filename)) {
                                auto size = std::filesystem::file_size(cache_filename);
                                spdlog::info("文件验证: 大小 {} 字节", size);
                            } else {
                                spdlog::error("文件写入后不存在: {}", cache_filename);
                            }
                        } else {
                            spdlog::error("无法打开文件: {}", cache_filename);
                        }
                    } else {
                        spdlog::warn("没有特征数据需要保存");
                    }
                } catch (const std::exception& e) {
                    spdlog::error("保存特征数据时出错: {}", e.what());
                }
            }

            bars[1].set_option(mpb::option::PrefixText{module_name + ""});
            bars[1].mark_as_completed();
            bars[0].tick();
            spdlog::info("[update] plugin={}, end", module_name);
        }

        bars[0].mark_as_completed();
        // 恢复终端光标显示
        mpb::show_console_cursor(true);
        return int(count);
    }

    void update_all() {
        std::string today = api::today();
        std::string last_trading_day = exchange::last_trading_day().only_date();
        std::string current_time = exchange::timestamp::now().toString(timeLayoutOfPhase).substr(0, 8);
        bool should_update = false;
        exchange::timestamp update_phase{};
        // 判断更新时机
        if (today == last_trading_day) { // 交易日
            for (const auto& trigger_time : allDateUpdateTimes) {
                if (current_time >= trigger_time) {
                    update_phase = exchange::timestamp::parse_time(trigger_time);
                    should_update = checkUpdateState(today, update_phase);
                    if (should_update) break;
                }
            }
        } else { // 非交易日
            if (current_time >= lastUpdateTime) {
                update_phase = lastUpdateTime;
                should_update = checkUpdateState(today, update_phase);
            }
        }

        // 执行更新
        if (should_update && !update_phase.empty()) {
            //factors::SwitchDate(cache::DefaultCanReadDate());
            auto all_action = cache::Plugins();
            update_with_adapters(all_action);
            doneUpdate(today, update_phase);
        }
    }
}
