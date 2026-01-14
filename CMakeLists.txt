cmake_minimum_required(VERSION 3.23)
project(vanitas LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(vanitas
  src/main.cpp
  src/normalizer.cpp
  src/classifier.cpp
  src/block_builder.cpp
  src/profile.cpp
  src/profile_manager.cpp
  src/args_parser.cpp
)

target_include_directories(vanitas PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

add_subdirectory(src/cli)
