#pragma once

#include <iosfwd>
#include <string_view>

namespace bacpipe {

class Logger {
  public:
    static void info(std::string_view message);
    static void warning(std::string_view message);
    static void error(std::string_view message);

    static void step(std::string_view message);
    static void command(std::string_view message);
    static void skip(std::string_view message);

  private:
    static void write(std::ostream &stream, std::string_view prefix, std::string_view message);
};

} // namespace bacpipe
