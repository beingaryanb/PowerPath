
cmake_minimum_required(VERSION 3.15)

project(PowerPath)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# JSON Library
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

FetchContent_MakeAvailable(json)

# HTTP Server Library
FetchContent_Declare(
    httplib
    URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.3.tar.gz
)

FetchContent_MakeAvailable(httplib)

# Include Directories
include_directories(include)

# Backend Executable
add_executable(
    powerpath
    src/main.cpp
)

# Link Libraries
target_link_libraries(
    powerpath
    PRIVATE
    nlohmann_json::nlohmann_json
    httplib::httplib
)

# Windows Socket Library
if(WIN32)
    target_link_libraries(powerpath PRIVATE ws2_32)
endif()

message(STATUS "PowerPath configuration complete!")
