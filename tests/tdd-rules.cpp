#include <quant1x/test/test.h>

#include <capnp/message.h>

#include "quant1x/engine/rule-context.h"
#include "quant1x/engine/rule-engine.h"
#include "quant1x/engine/rule-error.h"


engine::RuleError ExampleBaseRule(const engine::RuleContext& ctx) {
    if (ctx.snapshot->getPrice() > 100.0) {
        return engine::RuleError::INVALID_PRICE;
    }
    return engine::RuleError::OK;
}

TEST_CASE("rule-test", "[rules]") {
    auto engine = engine::RuleEngine::GetInstance();

    // 注册规则
    engine->RegisterRule(1, "基础规则-价格检查", ExampleBaseRule);

    // 构造参数和快照
    engine::RuleParameter param{};
    param.ignore_rule_group = {}; // 不忽略任何规则

    capnp::MallocMessageBuilder message;
    auto list = message.initRoot<QuoteList>();
    auto snapshots = list.initSnapshots(1);
    Snapshot::Builder snapshot = snapshots[0];
    snapshot.setPrice(1000.30);
    //Snapshot::Builder snapshot = {};
    //auto snapshot = Snapshot::Builder{};

    // 执行规则过滤
    auto [passed, err] = engine->Filter(param, &snapshot);
    if (err == engine::RuleError::OK) {
        std::cout << "通过的规则数: " << passed.size() << std::endl;
    } else {
        std::cerr << "规则失败, 错误码: " << engine::to_string(err) << std::endl;
    }

    // 打印规则列表
    engine->PrintRules();
}