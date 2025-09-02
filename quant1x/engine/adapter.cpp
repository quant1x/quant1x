#include <quant1x/engine/action.h>
#include <map>
#include <mutex>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <unordered_set>
#include <quant1x/runtime/config.h>
#include <filesystem>

namespace cache {

    namespace {
        std::mutex &getPluginMutex() {
            static std::mutex m;
            return m;
        }

        std::map<Kind, std::unique_ptr<DataAdapter>> &getPluginMap() {
            static std::map<Kind, std::unique_ptr<DataAdapter>> map;
            return map;
        }
    }

    DataAdapter *GetDataAdapter(Kind kind) {
        std::lock_guard<std::mutex> lock(getPluginMutex());
        auto &mapDataPlugins = getPluginMap();
        auto it = mapDataPlugins.find(kind);
        if (it != mapDataPlugins.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void Register(std::unique_ptr<DataAdapter> plugin) {
        std::lock_guard<std::mutex> lock(getPluginMutex());
        auto &mapDataPlugins = getPluginMap();
        auto it = mapDataPlugins.find(plugin->Kind());
        if (it != mapDataPlugins.end()) {
            throw ErrAlreadyExists();
        }
        mapDataPlugins[plugin->Kind()] = std::move(plugin);
    }

    std::vector<DataAdapter *> PluginsWithName(Kind pluginType, const std::vector<std::string> &keywords) {
        std::lock_guard<std::mutex> lock(getPluginMutex());

        if (keywords.empty()) {
            return {};
        }

        std::unordered_set<std::string> keywordSet(keywords.begin(), keywords.end());
        std::vector<std::pair<Kind, DataAdapter *>> candidates;
        auto &mapDataPlugins = getPluginMap();
        for (const auto &[kind, plugin]: mapDataPlugins) {
            if ((kind & pluginType) == pluginType && keywordSet.count(plugin->Key())) {
                candidates.emplace_back(kind, plugin.get());
            }
        }

        if (candidates.empty()) {
            return {};
        }

        std::sort(candidates.begin(), candidates.end(),
                  [](const auto &a, const auto &b) { return a.first < b.first; });

        std::vector<DataAdapter *> list;
        list.reserve(candidates.size());
        for (const auto &[_, ptr]: candidates) {
            list.push_back(ptr);
        }

        return list;
    }

    std::vector<DataAdapter *> Plugins(Kind mask) {
        std::lock_guard<std::mutex> lock(getPluginMutex());

        std::vector<DataAdapter *> result;

        for (const auto &[kind, plugin]: getPluginMap()) {
            if (mask == 0 || ((kind & mask) == mask)) {
                result.push_back(plugin.get());
            }
        }

        // 按 kind 排序（升序）
        std::sort(result.begin(), result.end(),
                  [](DataAdapter *a, DataAdapter *b) {
                      return a->Kind() < b->Kind();
                  });

        return result;
    }

    namespace fs = std::filesystem;
    constexpr const char *const cache1dPrefix = "flash";

    std::string FeatureAdapter::Filename(const exchange::timestamp &timestamp) {
        const std::string& key = Key();
        // 查找分隔符
        size_t pos = key.find('/');
        std::string cachePath, actualKey;
        bool found = (pos != std::string::npos);

        if (found) {
            cachePath = key.substr(0, pos);
            actualKey = key.substr(pos + 1);
        } else {
            actualKey = key;
            cachePath = cache1dPrefix;
        }

        // 构建完整路径
        fs::path fullPath = fs::path(config::default_cache_path()) / cachePath;
        std::string date = timestamp.only_date();
        std::string year = date.substr(0, 4);

        // 构建文件名
        fullPath /= year;
        fullPath /= actualKey + "." + date;

        // 转换为字符串并返回
        return fullPath.string();
    }
}