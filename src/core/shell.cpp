#include "bacpipe/core/shell.hpp"

namespace bacpipe {

std::string shell_quote(const std::string_view value) {
    std::string result{"'"};

    for (const char char_ : value) {
        if (char_ == '\'') {
            result += "'\\''";
        } else {
            result += char_;
        }
    }
    result += "'";
    return result;
}

std::string join_shell_args(const std::vector<std::string> &args) {
    std::string result{};

    for (const std::string &arg : args) {
        result += " ";
        result += shell_quote(arg);
    }

    return result;
}

} // namespace bacpipe
