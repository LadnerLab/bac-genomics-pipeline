#include "bacpipe/core/shell.hpp"

namespace bacpipe {

std::string shell_quote(std::string_view value) {
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

} // namespace bacpipe
