#include <quant1x/test/test.h>
#include <quant1x/std/numeric.h>

TEST_CASE("numeric::number_range<T> default constructor works", "[numerics]") {
    numeric::number_range<int> r;

    CHECK(r.min_ == std::numeric_limits<int>::lowest());
    CHECK(r.max_ == std::numeric_limits<int>::max());
}

TEST_CASE("numeric::number_range<T> two-argument constructor works", "[numeric::number_range]") {
    numeric::number_range<double> r(10.5, 20.5);

    CHECK(r.min_ == 10.5);
    CHECK(r.max_ == 20.5);
}

TEST_CASE("numeric::number_range<T> one-argument constructor works", "[numeric::number_range]") {
    numeric::number_range<float> r(3.14f);

    CHECK(r.min_ == 3.14f);
    CHECK(r.max_ == std::numeric_limits<float>::max());
}

TEST_CASE("numeric::number_range<T> string constructor: no separator", "[numeric::number_range]") {
    numeric::number_range<int> r("123");

    CHECK(r.min_ == 123);
    CHECK(r.max_ == std::numeric_limits<int>::max());
}

TEST_CASE("numeric::number_range<T> string constructor: leading separator", "[numeric::number_range]") {
    numeric::number_range<int> r("~456");

    CHECK(r.min_ == std::numeric_limits<int>::lowest());
    CHECK(r.max_ == 456);
}

TEST_CASE("numeric::number_range<T> string constructor: trailing separator", "[numeric::number_range]") {
    numeric::number_range<int> r("789~");

    CHECK(r.min_ == 789);
    CHECK(r.max_ == std::numeric_limits<int>::max());
}

TEST_CASE("numeric::number_range<T> string constructor: both empty", "[numeric::number_range]") {
    numeric::number_range<int> r("~");

    CHECK(r.min_ == std::numeric_limits<int>::lowest());
    CHECK(r.max_ == std::numeric_limits<int>::max());
}

TEST_CASE("numeric::number_range<T> string constructor: invalid input", "[numeric::number_range]") {
    numeric::number_range<int> r("invalid");

    CHECK(r.min_ == std::numeric_limits<int>::lowest());
    CHECK(r.max_ == std::numeric_limits<int>::max());
}

TEST_CASE("numeric::number_range<T> string constructor: min and max specified", "[numeric::number_range]") {
    numeric::number_range<int> r("100~200");

    CHECK(r.min_ == 100);
    CHECK(r.max_ == 200);
}

#include <yaml-cpp/yaml.h>
using namespace numeric;

struct number_range_yaml_parser {
    template <typename T>
    static void parse(const YAML::Node& node, number_range<T>& range) {
        if (node.IsScalar()) {
            range = number_range<T>(node.as<std::string>());
        } else if (node.IsMap()) {
            if (node["min"]) range.min_ = node["min"].as<T>();
            if (node["max"]) range.max_ = node["max"].as<T>();
        }
    }
};


template <typename T>
void parse_yaml_range(const YAML::Node& node, const std::string& key, number_range<T>& out) {
    if (!node[key]) return;

    if (node[key].IsScalar()) {
        auto range_str = node[key].as<std::string>();
        out = number_range<T>(range_str);
    } else if (node[key].IsMap()) {
        out = node[key].as<number_range<T>>();
    }
}

// 重载 operator >>
template <typename T>
struct YAML::convert<number_range<T>> {
    static Node encode(const number_range<T>& rhs) {
        Node node;
        node["min"] = rhs.min_;
        node["max"] = rhs.max_;
        return node;
    }

    static bool decode(const Node& node, number_range<T>& rhs) {
        number_range_yaml_parser::parse(node, rhs);
        return true;
    }
};

struct my_config {
    number_range<int> range;
    int other_field;
};

namespace YAML {
    template<>
    struct convert<my_config> {
        static bool decode(const Node& node, my_config& rhs) {
            parse_yaml_range(node, "range", rhs.range);
            if (node["other_field"]) {
                rhs.other_field = node["other_field"].as<int>();
            }
            return true;
        }
    };
}

TEST_CASE("number_range<T> can be parsed from scalar string", "[yaml]") {
    YAML::Node config = YAML::Load("range: \"100~200\"");
    my_config cfg = config.as<my_config>();

    CHECK(cfg.range.min_ == 100);
    CHECK(cfg.range.max_ == 200);
}

TEST_CASE("number_range<T> can be parsed from map style", "[yaml]") {
    YAML::Node config = YAML::Load(R"(
        range:
          min: 50
          max: 150
        other_field: 999
    )");

    my_config cfg = config.as<my_config>();

    CHECK(cfg.range.min_ == 50);
    CHECK(cfg.range.max_ == 150);
    CHECK(cfg.other_field == 999);
}

TEST_CASE("number_range<T> uses default if not present", "[yaml]") {
    YAML::Node config = YAML::Load("other_field: 123");
    my_config cfg = config.as<my_config>();

    CHECK(cfg.range.min_ == std::numeric_limits<int>::lowest());
    CHECK(cfg.range.max_ == std::numeric_limits<int>::max());
    CHECK(cfg.other_field == 123);
}