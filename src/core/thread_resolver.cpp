#include "bacpipe/core/thread_resolver.hpp"

#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>
#include <thread>

namespace bacpipe {
namespace {

std::optional<std::uint32_t> parse_pos_int(std::string_view text) {
    std::uint32_t result{};
    const auto *begin = text.data();
    const auto *end = text.data() + text.size();

    const auto [ptr, err] = std::from_chars(begin, end, result);
    if (err != std::errc{} || ptr != end || result == 0) {
        return std::nullopt;
    }
    return result;
}

std::optional<std::uint32_t> read_positive_env(const char *name) {
    const char *value = std::getenv(name);
    if (value == nullptr) {
        return std::nullopt;
    }
    return parse_pos_int(value);
}

} // namespace

std::uint32_t ThreadResolver::resolve_default() {
    // get number of SLURM-allocated threads
    if (const auto slurm_threads = read_positive_env("SLURM_CPUS_PER_TASK")) {
        return *slurm_threads;
    }

    // get number of hardware threads
    const auto hardware_threads = std::thread::hardware_concurrency();
    if (hardware_threads == 0) {
        return 1;
    }

    return hardware_threads;
}

} // namespace bacpipe