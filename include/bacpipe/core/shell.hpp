#pragma once

#include <string>
#include <string_view>

namespace bacpipe {

[[nodiscard]] std::string shell_quote(std::string_view value);

} // namespace bacpipe
