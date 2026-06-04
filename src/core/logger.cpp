#include "bacpipe/core/logger.hpp"

#include <iostream>

namespace bacpipe {

void Logger::info(std::string_view message) {
    write(std::cout, "[info] ", message);
}

void Logger::warning(std::string_view message) {
    write(std::cout, "[warning] ", message);
}

void Logger::error(std::string_view message) {
    write(std::cerr, "[error] ", message);
}

void Logger::step(std::string_view message) {
    write(std::cout, "[step] ", message);
}

void Logger::command(std::string_view message) {
    write(std::cout, "[command] ", message);
}

void Logger::skip(std::string_view message) {
    write(std::cout, "[skip] ", message);
}

void Logger::write(std::ostream &stream, std::string_view prefix, std::string_view message) {
    stream << prefix << message << '\n';
}

} // namespace bacpipe
