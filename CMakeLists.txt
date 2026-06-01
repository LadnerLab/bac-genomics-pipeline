cmake_minimum_required(VERSION 3.20)

project(
    bac_genomics_pipeline
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(bacpipe
    src/main.cpp
    src/application.cpp
    src/core/runner.cpp
    src/core/thread_resolver.cpp
    src/pipeline/pipeline_step.cpp
)

target_include_directories(bacpipe PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_features(bacpipe PRIVATE cxx_std_23)

set_target_properties(bacpipe PROPERTIES CXX_EXTENSIONS OFF)