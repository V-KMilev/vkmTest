# A library in a vkm library's shape, for vkm_test_consumer, vkm_test_flags and vkm_test_includes to pass: it stops without the target it requires, its asserts are on, its sources compiling with -UNDEBUG whatever the build type, and its public headers include no private one.
cmake_minimum_required(VERSION 3.25)
project(vkm_fixture LANGUAGES CXX)

if(NOT TARGET vkm_fixture_needed)
    message(FATAL_ERROR "vkm_fixture requires the target 'vkm_fixture_needed', which the consuming project must add first.")
endif()

add_library(vkm_fixture STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/vkm_fixture/half.cpp)
target_include_directories(vkm_fixture
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private
)
target_link_libraries(vkm_fixture PUBLIC vkm_fixture_needed)
target_compile_options(vkm_fixture PRIVATE -UNDEBUG)
