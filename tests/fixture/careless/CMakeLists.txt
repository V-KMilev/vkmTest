# A library that breaks the shape vkm_test_consumer and vkm_test_flags check: it configures without the target it requires, its asserts go as the build type says, and it calls Tracy.
cmake_minimum_required(VERSION 3.25)
project(vkm_careless LANGUAGES CXX)

add_library(vkm_careless STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/vkm_careless/half.cpp)
