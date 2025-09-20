#include <iostream>
#include "strings.h"

int main() {
    std::string test = "XMLHttpRequest";
    std::string result = strings::snake_case(test);
    std::cout << "Input: " << test << std::endl;
    std::cout << "Result: " << result << std::endl;
    std::cout << "Expected: xml_http_request" << std::endl;
    return 0;
}