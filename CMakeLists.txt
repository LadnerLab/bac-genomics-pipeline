cmake_minimum_required(VERSION 3.20)

project(
    bac_genomics_pipeline
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(tomlplusplus CONFIG REQUIRED)

add_executable(bacpipe
    src/main.cpp
    src/application.cpp

    src/config/config_loader.cpp
    src/config/path_expander.cpp

    src/core/file_discovery.cpp
    src/core/logger.cpp
    src/core/path_builder.cpp
    src/core/runner.cpp
    src/core/shell.cpp
    src/core/thread_resolver.cpp

    src/pipeline/pipeline_step.cpp
    src/pipeline/trim.cpp
    src/pipeline/assemble.cpp
    src/pipeline/autocycler.cpp
    src/pipeline/circularize.cpp
    src/pipeline/medaka.cpp
)

target_include_directories(bacpipe PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_features(bacpipe PRIVATE cxx_std_23)

set_target_properties(bacpipe PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bacpipe PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(bacpipe PRIVATE tomlplusplus::tomlplusplus)
