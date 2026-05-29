cmake_minimum_required(VERSION 3.20)

project(
    bac_genomics_pipeline
    VERSION 0.1.0
    LANGUAGES CXX
)

add_executable(bacpipe
    src/main.cpp
)

target_compile_features(bacpipe PRIVATE cxx_std_23)

set_target_properties(bacpipe PROPERTIES
    CXX_EXTENSIONS OFF
)