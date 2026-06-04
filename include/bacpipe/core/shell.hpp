#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace bacpipe {

std::string shell_quote(const std::string_view value);
std::string join_shell_args(const std::vector<std::string> &args);

} // namespace bacpipe
