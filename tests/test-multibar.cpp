#include <q1x/std/api.h>
#include <chrono>
#include <indicators/progress_bar.hpp>
#include <thread>
#include <indicators/dynamic_progress.hpp>

int main() {
    indicators::DynamicProgress<indicators::ProgressBar> bars;
    indicators::ProgressBar bar{indicators::option::BarWidth{50},
                                indicators::option::Start{"["},
                                indicators::option::Fill{"="},
                                indicators::option::Lead{">"},
                                indicators::option::Remainder{" "},
                                indicators::option::End{" ]"},
                                indicators::option::PostfixText{"Getting started"},
                                indicators::option::ForegroundColor{indicators::Color::green},
                                indicators::option::FontStyles{
                                    std::vector<indicators::FontStyle>{indicators::FontStyle::bold}},
                                indicators::option::ShowPercentage{true},
                                indicators::option::ShowSpeed{true},
                                indicators::option::ShowElapsedTime{true},
                                indicators::option::ShowRemainingTime{true},
                                //indicators::option::Sho
    };
    bars.push_back(bar);
    int64_t count = 10000 * 10000;
    // Update bar state
    bars[0].set_option(indicators::option::MaxProgress{count});
    int64_t i = 0;
    while (true) {
        bars[0].set_option(indicators::option::PrefixText{ fmt::format("[{}/{}]", ++i, count)});
        bars[0].tick();
        if (bars[0].is_completed()) {
            break;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}