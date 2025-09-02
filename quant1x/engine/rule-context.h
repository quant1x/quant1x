#pragma once
#ifndef QUANT1X_RULE_CONTEXT_H
#define QUANT1X_RULE_CONTEXT_H 1

#include <quant1x/proto/snapshot.capnp.h>
#include <string>
#include <map>
#include <any>
#include <vector>

namespace engine {

    struct RuleParameter {
        std::vector<int> ignore_rule_group;
    };

    struct RuleContext {
        RuleParameter param;
        Snapshot::Builder* snapshot;
        std::map<std::string, std::any> metadata; // 扩展字段
    };

} // namespace engine

#endif //QUANT1X_RULE_CONTEXT_H
