#include "bacpipe/application.hpp"

#include <span>

int main(int argc, char *argv[]) {
    const bacpipe::Application app{std::span<char *>{argv, static_cast<std::size_t>(argc)}};
    return app.run();
}