# A project that discovers a binary's tests as a suite's CMakeLists.txt does, for tests/discovery.cmake to read back from ctest. The binary is imported, so nothing is built:
#
#   -DVKM_DISCOVER=<file>    cmake/discover.cmake
#   -DVKM_BINARY=<file>      the binary
#   -DVKM_ARGUMENTS=<a|b>    vkm_discover_tests' arguments after the suite's name
cmake_minimum_required(VERSION 3.25)
project(vkm_discovery LANGUAGES NONE)

include(${VKM_DISCOVER})
enable_testing()

add_executable(suite IMPORTED)
set_target_properties(suite PROPERTIES IMPORTED_LOCATION ${VKM_BINARY})

string(REPLACE "|" ";" _arguments "${VKM_ARGUMENTS}")
vkm_discover_tests(suite vkm_fixture ${_arguments})
