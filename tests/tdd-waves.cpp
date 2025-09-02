#include <quant1x/test/test.h>
#include <quant1x/ta/waves.h>

using namespace ta::waves;

TEST_CASE("Empty input", "[detect]") {
    std::vector<double> high, low;
    auto [peaks, valleys] = detect_peaks_and_valleys(std::span<const double>{high}, std::span<const double>{low});
    REQUIRE(peaks.empty());
    REQUIRE(valleys.empty());
}

TEST_CASE("Single point", "[detect]") {
    std::vector<double> high = {3.0}, low = {1.0};
    auto [peaks, valleys] = detect_peaks_and_valleys(high, low);
    REQUIRE(peaks == std::vector<int>{0});
    REQUIRE(valleys == std::vector<int>{0});
}

TEST_CASE("Simple peak and valley", "[detect]") {
    std::vector<double> high = {1, 10, 2, 6, 4, 5, 3, 8, 5, 7, 3, 10, 5};
    std::vector<double> low = {0, 8, 0, 4, 2, 3, 1, 6, 3, 5, 1, 8, 3};

    auto [peaks, valleys] = detect_peaks_and_valleys(std::span<const double>{high}, std::span<const double>{low});
    REQUIRE(peaks == std::vector<int>{1, 11});
    REQUIRE(valleys == std::vector<int>{0, 2, 12});
}

TEST_CASE("Template supports float and int", "[template]") {
    // float
    std::vector<float> high_f = {1.0f, 3.0f, 2.0f};
    std::vector<float> low_f = {1.0f, 2.0f, 1.0f};
    auto [p1, v1] = detect_peaks_and_valleys(std::span<const float>{high_f}, std::span<const float>{low_f});
    REQUIRE(!p1.empty());
    REQUIRE(!v1.empty());

    // int
    std::vector<int> high_i = {1, 3, 2, 4, 3};
    std::vector<int> low_i = {1, 2, 1, 3, 2};
    auto [p2, v2] = detect_peaks_and_valleys(std::span<const int>{high_i}, std::span<const int>{low_i});
    REQUIRE(p2 == std::vector<int>{1, 3});
    REQUIRE(v2 == std::vector<int>{2});
}

TEST_CASE("Refine peaks: no valley between -> keep higher", "[refine]") {
    std::vector<int> peaks = {0, 2};
    std::vector<int> valleys = {}; // 无谷
    std::vector<double> high = {2.0, 1.0, 3.0};

    auto result = refine_peaks_by_valleys(peaks, valleys, std::span<const double>{high});
    REQUIRE(result == std::vector<int>{2}); // 3.0 > 2.0
}

TEST_CASE("Normalize: consecutive peaks -> keep higher", "[normalize]") {
    std::vector<int> peaks = {0, 1};
    std::vector<int> valleys = {};
    std::vector<double> high = {2.0, 3.0}, low = {1.0, 1.0};

    auto [p, v] = normalize_peaks_and_valleys(peaks, valleys, std::span<const double>{high}, std::span<const double>{low});
    REQUIRE(p == std::vector<int>{1});
}

TEST_CASE("segments", "[waves-struct]") {
    std::vector<double> high_list = {2, 9, 1, 5, 3, 4, 2, 7, 4, 6, 2, 9, 4};
    std::vector<double> low_list = {2, 9, 1, 5, 3, 4, 2, 7, 4, 6, 2, 9, 4};

    auto segments = detect_complete_wave_structure(high_list,low_list);

    std::cout << "完整波浪结构（按层级排序）:\n";
    for (const auto& seg : segments) {
        std::string trend = seg.is_rising ? "↑" : "↓";
        std::cout << "  L" << seg.level
                  << " [" << seg.start << " → " << seg.end << "] "
                  << trend << "\n";
    }
}