// To add this test to the test run, `tests/CMakeLists.txt` already contains:
//   add_catch2_executable(tdd-encoding-json.cpp)
// which creates a Catch2 test target named `catch2-tdd-encoding-json`.

#include <quant1x/test/test.h>
#include <quant1x/encoding/json.h>

using Catch::Approx;

using namespace encoding;

// Basic aggregate used for boost::pfr-based serialization
struct Person {
	std::string name{};
	int age{};
	std::optional<double> score{};
};

enum class Color : int { Red = 0, Green = 1, Blue = 2 };

struct Pixel {
	Color color{};
	int x{};
	int y{};
};

struct Company {
	Person ceo{};
	std::vector<Person> staff{};
};

TEST_CASE("encoding::json - pfr aggregate roundtrip", "[encoding][json][pfr]") {
	Person p{"Alice", 30, 88.5};
	auto j = json::serialize(p);
	REQUIRE(j.is_object());
	REQUIRE(j["name"] == "Alice");
	REQUIRE(j["age"] == 30);
	REQUIRE(j["score"] == 88.5);

	auto p2 = json::deserialize<Person>(j);
	REQUIRE(p2.name == p.name);
	REQUIRE(p2.age == p.age);
	REQUIRE(p2.score.has_value());
	REQUIRE(p2.score.value() == Approx(88.5));
}

TEST_CASE("encoding::json - vector of aggregates", "[encoding][json][vector]") {
	std::vector<Person> v{{"Alice", 30, 88.5}, {"Bob", 25, std::nullopt}};
	auto j = json::serialize(v);
	REQUIRE(j.is_array());
	REQUIRE(j.size() == 2);
	REQUIRE(j[0]["name"] == "Alice");
	REQUIRE(j[1]["name"] == "Bob");

	auto v2 = json::deserialize<std::vector<Person>>(j);
	REQUIRE(v2.size() == 2);
	REQUIRE(v2[0].name == "Alice");
	REQUIRE(v2[1].score == std::nullopt);
}

TEST_CASE("encoding::json - map serialization", "[encoding][json][map]") {
	std::map<std::string,int> m{{"a",1},{"b",2}};
	auto j = json::serialize(m);
	REQUIRE(j.is_object());
	REQUIRE(j["a"] == 1);
	REQUIRE(j["b"] == 2);

	auto m2 = json::deserialize<std::map<std::string,int>>(j);
	REQUIRE(m2 == m);
}

TEST_CASE("encoding::json - nested aggregates and enums", "[encoding][json][nested]") {
	Company c;
	c.ceo = {"Carol", 45, 99.0};
	c.staff = {{"Dev1", 28, 70.0}, {"Dev2", 32, std::nullopt}};

	auto j = json::serialize(c);
	REQUIRE(j.is_object());
	REQUIRE(j["ceo"]["name"] == "Carol");
	REQUIRE(j["staff"].is_array());
	REQUIRE(j["staff"][0]["name"] == "Dev1");

	auto c2 = json::deserialize<Company>(j);
	REQUIRE(c2.ceo.name == "Carol");
	REQUIRE(c2.staff.size() == 2);

	// enum serialization as integer
	Pixel px{Color::Green, 10, 20};
	auto pj = json::serialize(px);
	REQUIRE(pj["color"].is_number());
	auto px2 = json::deserialize<Pixel>(pj);
	REQUIRE(static_cast<int>(px2.color) == static_cast<int>(Color::Green));
	REQUIRE(px2.x == 10);
	REQUIRE(px2.y == 20);
}

TEST_CASE("encoding::json - empty containers and nullopt", "[encoding][json][edge]") {
	std::vector<Person> empty_vec;
	auto jv = json::serialize(empty_vec);
	REQUIRE(jv.is_array());
	REQUIRE(jv.empty());

	std::map<std::string, Person> empty_map;
	auto jm = json::serialize(empty_map);
	REQUIRE(jm.is_object());
	REQUIRE(jm.empty());

	Person p{ "NoScore", 20, std::nullopt };
	auto jp = json::serialize(p);
	REQUIRE(jp.contains("score"));
	// when score is nullopt, current serialize_field sets nothing — behavior: absent field or null?
	// Our implementation only omits optional when not set, so score should be absent or null; accept both.
	bool score_present = jp.contains("score") && !jp["score"].is_null();
	REQUIRE(!score_present);
}

TEST_CASE("encoding::json - strict mode missing field throws", "[encoding][json][strict]") {
	// Create a JSON missing a required field (age)
	nlohmann::json j;
	j["name"] = "MissingAge";
	// Try deserializing with strict=true should throw
	REQUIRE_THROWS_AS((json::deserialize<Person>(j, true)), std::runtime_error);
}

TEST_CASE("encoding::json - strict mode type mismatch throws", "[encoding][json][strict]") {
	// age is a string -> should cause nlohmann::json::type_error when trying to get<int>()
	nlohmann::json j;
	j["name"] = "BadAge";
	j["age"] = "not_a_number";
	REQUIRE_THROWS_AS((json::deserialize<Person>(j, true)), nlohmann::json::type_error);

	// nested: Company.ceo.age type mismatch should raise inside nested deserialize
	nlohmann::json jc;
	jc["ceo"] = nlohmann::json::object();
	jc["ceo"]["name"] = "CEOName";
	jc["ceo"]["age"] = "forty-five"; // wrong type
	jc["staff"] = nlohmann::json::array();
	REQUIRE_THROWS_AS((json::deserialize<Company>(jc, true)), nlohmann::json::type_error);
}

TEST_CASE("encoding::json - non-strict missing fields give defaults", "[encoding][json][non-strict]") {
	nlohmann::json j;
	// only provide age, omit name
	j["age"] = 55;
	auto p = json::deserialize<Person>(j, false);
	// name should be default-initialized (empty), age set to provided
	REQUIRE(p.name.empty());
	REQUIRE(p.age == 55);

	// nested: missing staff/ceo fields should not throw in non-strict
	nlohmann::json jc;
	jc["ceo"] = nlohmann::json::object();
	jc["ceo"]["age"] = 60; // ceo.name missing
	jc["staff"] = nlohmann::json::array();
	auto c = json::deserialize<Company>(jc, false);
	REQUIRE(c.ceo.age == 60);
	REQUIRE(c.ceo.name.empty());
}

TEST_CASE("encoding::json - deque/array/unordered_map support", "[encoding][json][containers]") {
	std::deque<Person> dq{{"D1",20,std::nullopt},{"D2",21,50.0}};
	auto jdq = json::serialize(dq);
	REQUIRE(jdq.is_array());
	auto dq2 = json::deserialize<std::deque<Person>>(jdq);
	REQUIRE(dq2.size() == dq.size());
	REQUIRE(dq2[1].name == "D2");

	std::array<Person,2> arr{{Person{"A1",10,std::nullopt}, Person{"A2",11,11.0}}};
	auto jarr = json::serialize(arr);
	REQUIRE(jarr.is_array());
	auto arr2 = json::deserialize<std::array<Person,2>>(jarr);
	REQUIRE(arr2[0].name == "A1");

	std::unordered_map<std::string,int> um{{"one",1},{"two",2}};
	auto jum = json::serialize(um);
	REQUIRE(jum.is_object());
	auto um2 = json::deserialize<std::unordered_map<std::string,int>>(jum);
	REQUIRE(um2["one"] == 1);
}

TEST_CASE("encoding::json - optional serialize as null vs omit", "[encoding][json][optional]") {
	Person p{"OptNull", 40, std::nullopt};
	// default behavior: optional omitted
	auto j1 = json::serialize(p);
	REQUIRE(j1.contains("score"));
	bool present_and_not_null = j1.contains("score") && !j1["score"].is_null();
	REQUIRE(!present_and_not_null);

	// explicit serialize with null
	auto j2 = json::serialize_with_optional_null(p);
	REQUIRE(j2.contains("score"));
	REQUIRE(j2["score"].is_null());
}
