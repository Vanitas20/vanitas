cmake_minimum_required(VERSION 3.23)
project(vanitas)

set(PROJECT_NAME vanitas)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SRC 
    src/normalizer.cpp
    src/classifier.cpp
    src/block_builder.cpp
    src/profile.cpp
    src/profile_manager.cpp
)


include(FetchContent)
include_directories(${PROJECT_NAME} src/include)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)

FetchContent_Declare(
  toml11
  GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
  GIT_TAG        v4.4.0
)

FetchContent_MakeAvailable(ftxui)
FetchContent_MakeAvailable(toml11)

add_executable(${PROJECT_NAME} src/main.cpp ${SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE
    ftxui::component
    ftxui::dom
    ftxui::screen

    toml11::toml11
)
