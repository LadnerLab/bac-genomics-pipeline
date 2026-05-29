#pragma once

#include <cstdint>

namespace bacpipe {

class ThreadResolver {
  public:
    static std::uint32_t resolve_default();
};

} // namespace bacpipe