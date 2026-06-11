#include "status.h"

#include "session.h"

#include <filesystem>
#include <chrono>
#include <system_error>

namespace exchange {

    std::optional<timestamp> get_filename_modified_time(const std::string &fname) {
        try {
            namespace fs = std::filesystem;
            auto ftime = fs::last_write_time(fname);
            // Convert fs::file_time_type to system_clock::time_point
            using file_clock = decltype(ftime)::clock;
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - file_clock::now() + std::chrono::system_clock::now());
            return timestamp(std::chrono::system_clock::time_point(sctp));
        } catch (const std::filesystem::filesystem_error &) {
            return std::nullopt;
        }
    }

    bool should_initialize_file(const std::string &fname) {
        auto mt = get_filename_modified_time(fname);
        if (!mt.has_value()) {
            return true;
        }
        return exchange::can_initialize(mt);
    }

    bool should_update_file(const std::string &fname) {
        auto mt = get_filename_modified_time(fname);
        if (!mt.has_value()) {
            return true;
        }
        auto [canUpdate, status] = exchange::can_update_in_realtime(mt.value());
        return canUpdate;
    }

} // namespace exchange
