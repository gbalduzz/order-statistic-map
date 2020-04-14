cmake_minimum_required(VERSION 3.3)
project(RandomAccessMap CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

find_package(GTest)

if (GTEST_FOUND)
    enable_testing()
    add_subdirectory(test)
else()
    message("Gtest not found. Tests will not be built.")
endif ()

