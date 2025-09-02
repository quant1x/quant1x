#pragma once
#ifndef QUANT1X_RULE_ENGINE_H
#define QUANT1X_RULE_ENGINE_H 1

#include <bitset>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

#include "rule-context.h"
#include "rule-error.h"

namespace engine {

    using Kind = uint32_t;

    // 规则执行函数签名
    using RuleFunc = std::function<RuleError(const RuleContext &)>;

    // Rule 类封装
    class Rule {
    public:
        Rule(Kind kind, const std::string &name, RuleFunc func) : kind_(kind), name_(name), func_(func) {}

        Kind               GetKind() const { return kind_; }
        const std::string &GetName() const { return name_; }

        RuleError Execute(const RuleContext &ctx) { return func_(ctx); }

    private:
        Kind        kind_;
        std::string name_;
        RuleFunc    func_;
    };

    // RuleEngine 单例类
    class RuleEngine {
    public:
        using Ptr = std::shared_ptr<RuleEngine>;

        static Ptr GetInstance() {
            static Ptr instance = std::make_shared<RuleEngine>();
            return instance;
        }

        RuleEngine()  = default;
        ~RuleEngine() = default;

        // 注册规则
        bool RegisterRule(Kind kind, const std::string &name, RuleFunc func);

        // 过滤并执行规则
        std::pair<std::vector<uint64_t>, RuleError> Filter(const RuleParameter &param, Snapshot::Builder* snapshot);

        // 打印所有规则
        void PrintRules() const;

    private:
        std::map<Kind, std::shared_ptr<Rule>> rules_;
        mutable std::shared_mutex             mutex_;
        mutable std::vector<Kind>             sorted_kinds_cache_;
        void                                  UpdateSortedCache() const;
    };

}  // namespace engine

#endif  // QUANT1X_RULE_ENGINE_H
