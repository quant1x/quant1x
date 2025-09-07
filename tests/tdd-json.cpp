#include <quant1x/test/test.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

TEST_CASE("json-basic", "[encoding]") {
    std::string json_str = R"(
        {
            "user": {
                "id": 1,
                "details": {
                    "email": "alice@example.com",
                    "roles": ["admin", "developer"]
                }
            }
        }
    )";

    json j = json::parse(json_str);

    std::cout << "User ID: " << j["user"]["id"] << std::endl;
    std::cout << "Email: " << j["user"]["details"]["email"] << std::endl;
    std::cout << "Roles: " << j["user"]["details"]["roles"].dump() << std::endl;
}