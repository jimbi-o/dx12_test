cmake_minimum_required(VERSION 3.15)
project(DirectX12Example LANGUAGES CXX)

set(SOURCES
  src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_options(${PROJECT_NAME} PUBLIC
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:Clang>:-Weverything -Wno-c++98-c++11-c++14-compat -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c++20-compat>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /nologo /FC>
)
target_compile_definitions(${PROJECT_NAME}
  PUBLIC
  $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>
)
target_link_libraries(${PROJECT_NAME}
    d3d12
    dxgi
    D3DCompiler
)
