#include "rule_engine.h"
#include <spdlog/spdlog.h>

namespace engine {

    void RuleEngine::UpdateSortedCache() const {
        sorted_kinds_cache_.clear();
        for (const auto& [kind, _] : rules_) {
            sorted_kinds_cache_.push_back(kind);
        }
        std::sort(sorted_kinds_cache_.begin(), sorted_kinds_cache_.end());
    }

    bool RuleEngine::RegisterRule(Kind kind, const std::string& name, RuleFunc func) {
        std::unique_lock lock(mutex_);
        if (rules_.find(kind) != rules_.end()) {
            spdlog::error("规则已存在");
            return false; // 已存在
        }
        rules_[kind] = std::make_shared<Rule>(kind, name, func);
        UpdateSortedCache(); // 更新缓存排序
        return true;
    }

    std::pair<std::vector<uint64_t>, RuleError>
    RuleEngine::Filter(const RuleParameter& param, Snapshot::Builder* snapshot) {
        std::shared_lock lock(mutex_);

        RuleContext ctx;
        ctx.param = param;
        ctx.snapshot = snapshot;

        std::bitset<1024> bitset; // 假设有最多1024个规则
        RuleError error = RuleError::OK;

        for (const auto& kind : sorted_kinds_cache_) {
            if (std::find(param.ignore_rule_group.begin(),
                          param.ignore_rule_group.end(),
                          static_cast<int>(kind)) != param.ignore_rule_group.end()) {
                continue;
            }

            auto it = rules_.find(kind);
            if (it == rules_.end()) {
                error = RuleError::UNKNOWN_RULE_KIND;
                spdlog::error("未找到规则类型：" + std::to_string(kind));
                break;
            }

            auto &rule = it->second;
            error = rule->Execute(ctx);
            if (error != RuleError::OK) {
                spdlog::error("规则失败: " + rule->GetName() + ", 错误: " + to_string(error));
                break;
            }
            bitset.set(kind);
        }

        std::vector<uint64_t> passed;
        for (size_t i = 0; i < bitset.size(); ++i) {
            if (bitset.test(i)) {
                passed.push_back(static_cast<uint64_t>(i));
            }
        }

        return {passed, error};
    }

    void RuleEngine::PrintRules() const {
        std::shared_lock lock(mutex_);
        spdlog::info("规则总数: " + std::to_string(rules_.size()));
        for (const auto& [kind, rule] : rules_) {
            spdlog::info("kind: " + std::to_string(rule->GetKind()) +
                         ", name: " + rule->GetName());
        }
    }

} // namespace engine