#pragma once
#ifndef QUANT1X_FACTORS_FACTORY_H
#define QUANT1X_FACTORS_FACTORY_H 1

#include <mutex>
#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <tsl/robin_map.h>
#include <quant1x/exchange/timestamp.h>
#include <quant1x/encoding/csv.h>
#include <quant1x/data/adapter.h>
#include <quant1x/runtime/once.h>
#include <quant1x/std/atomic.h>

namespace factors {

    template <typename T, typename Adapter>
    class FactorManager {
    private:
        struct DataCache {
            exchange::timestamp            date;
            tsl::robin_map<std::string, T> map;
        };

        struct Cache {
            std::shared_ptr<RollingOnce>      once;
            base::atomic_share_ptr<DataCache> data;

            Cache() {
                auto adapter = Adapter();
                // 每天 08:30 重置
                once = RollingOnce::create(adapter.Key(), 9, 0);
            }
        };

        static Cache &instance() {
            static Cache inst;
            return inst;
        }

    public:
        static std::optional<T> get(const std::string &code, const exchange::timestamp &timestamp) {
            auto &cache = instance();

            // 1. 尝试更新 (如果处于重置状态)
            // RollingOnce::Do 内部使用原子操作检查状态，只有在需要更新时才加锁
            cache.once->Do([&]() {
                auto adapter        = Adapter();
                exchange::timestamp align_date = timestamp.pre_market_time();
                auto cache_filename = adapter.Filename(align_date);

                auto new_data = std::make_shared<DataCache>();
                new_data->date = align_date;

                if (std::filesystem::exists(cache_filename)) {
                    std::vector<T> list = encoding::csv::csv_to_slices<T>(cache_filename);
                    for (auto const &v : list) {
                        new_data->map.insert_or_assign(v.Code, v);
                    }
                }
                // 原子替换数据，实现 Copy-On-Write，避免读锁
                cache.data.store(new_data);
            });

            // 2. 原子读取数据 (无锁)
            auto current_data = cache.data.load();
            if (current_data) {
                // 如果需要严格的日期匹配，可以在这里检查 current_data->date
                // 但考虑到 RollingOnce 的特性，这里默认返回缓存的数据
                auto it = current_data->map.find(code);
                if (it != current_data->map.end()) {
                    return it->second;
                }
            }
            return std::nullopt;
        }
    };
}

#endif // QUANT1X_FACTORS_FACTORY_H
