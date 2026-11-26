# A library that breaks the shape vkm_test_consumer, vkm_test_flags and vkm_test_includes check: it configures without the target it requires, its asserts go as the build type says, it calls Tracy, and a public header of its includes a private one.
cmake_minimum_required(VERSION 3.25)
project(vkm_careless LANGUAGES CXX)

add_library(vkm_careless STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/vkm_careless/half.cpp)
