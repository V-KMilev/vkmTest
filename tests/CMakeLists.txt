# vkm_test's own suite: TEST("what works") in tests/<topic>.cpp, each a ctest named "vkm_test: <what works>" and labeled vkm_test. What the harness makes of a test that fails, skips or throws is seen from outside: fixture/ holds a suite whose tests end each way on purpose, which run.cmake runs one at a time and discovery.cmake reads back through ctest, and two libraries, one in a vkm library's shape and one not, for the checks of a library as a consumer builds it.

file(GLOB _vkm_test_test_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(vkm_test_tests ${_vkm_test_test_sources})
target_link_libraries(vkm_test_tests PRIVATE ${PROJECT_NAME})
# As a library's suite whose asserts flag is on, which check.h's VKM_TEST_LIBRARY_ASSERTS reads.
target_compile_definitions(vkm_test_tests PRIVATE
    VKM_TEST_SUITE="vkm_test"
    VKM_TEST_ASSERTS_FLAG=1
)
vkm_discover_tests(vkm_test_tests vkm_test)

# The fixture suites, which no ctest runs as they are: one whose tests end each way, in several areas, and one the runner refuses.
add_executable(vkm_test_fixture
    ${CMAKE_CURRENT_SOURCE_DIR}/fixture/areas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fixture/ends.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fixture/hooks.cpp
)
add_executable(vkm_test_fixture_broken ${CMAKE_CURRENT_SOURCE_DIR}/fixture/broken.cpp)

# The fixture libraries: vkm_fixture requires vkm_fixture_needed and keeps its asserts; vkm_careless does neither, and calls Tracy.
add_library(vkm_fixture_needed INTERFACE)
add_subdirectory(fixture/library)
add_subdirectory(fixture/careless)

foreach(_target vkm_test_tests vkm_test_fixture vkm_test_fixture_broken vkm_fixture vkm_careless)
    target_compile_options(${_target} PRIVATE ${_vkm_test_warnings})
endforeach()
foreach(_target vkm_test_fixture vkm_test_fixture_broken)
    target_link_libraries(${_target} PRIVATE ${PROJECT_NAME})
endforeach()

# "vkm_test: <promise>", a run of a fixture binary that run.cmake checks: each keyword of the call is its -D<keyword>, a list joined by '|'.
function(_vkm_test_run promise)
    cmake_parse_arguments(PARSE_ARGV 1 _arg "" "BINARY;TEST;STATUS;KEPT;REMOVED" "SAYS;NOT_SAYS;ENVIRONMENT")
    string(REPLACE ";" "|" _says "${_arg_SAYS}")
    string(REPLACE ";" "|" _not_says "${_arg_NOT_SAYS}")
    string(REPLACE ";" "|" _environment "${_arg_ENVIRONMENT}")
    add_test(NAME "vkm_test: ${promise}"
        COMMAND ${CMAKE_COMMAND} "-DBINARY=$<TARGET_FILE:${_arg_BINARY}>" "-DTEST=${_arg_TEST}"
            -DSTATUS=${_arg_STATUS} "-DSAYS=${_says}" "-DNOT_SAYS=${_not_says}" "-DKEPT=${_arg_KEPT}"
            "-DREMOVED=${_arg_REMOVED}" "-DENVIRONMENT=${_environment}" -P ${CMAKE_CURRENT_SOURCE_DIR}/run.cmake
    )
    set_tests_properties("vkm_test: ${promise}" PROPERTIES LABELS vkm_test)
endfunction()

set(_vkm_test_folders $<TARGET_FILE_DIR:vkm_test_fixture>/test_scratch/fixture)

_vkm_test_run("a failed CHECK fails the test, prints both sides, and the checks after it still run"
    BINARY vkm_test_fixture TEST "fixture: a CHECK that fails, then another" STATUS 1
    SAYS "CHECK(one == two) with 1 == 2" "CHECK(two < one) with 2 < 1" "FAILED"
)
_vkm_test_run("a NaN part fails CHECK_NEAR"
    BINARY vkm_test_fixture TEST "fixture: a CHECK_NEAR on a value with a NaN part" STATUS 1
    SAYS "CHECK_NEAR(withNaN, zero) with (0, " "apart, more than 1" "CHECK_NEAR(nan, 0.0f) with " "FAILED"
)
_vkm_test_run("a failed REQUIRE stops the test, and tearDown still runs"
    BINARY vkm_test_fixture TEST "fixture: a REQUIRE that fails" STATUS 1
    SAYS "REQUIRE(one == two) with 1 == 2" "torn down fixture: a REQUIRE that fails" "FAILED"
    NOT_SAYS "after the REQUIRE"
)
_vkm_test_run("a skip exits 77 and says why"
    BINARY vkm_test_fixture TEST "fixture: a test that skips" STATUS 77
    SAYS "SKIPPED: no device here" NOT_SAYS "FAILED"
)
_vkm_test_run("a check that failed before a skip still fails the test"
    BINARY vkm_test_fixture TEST "fixture: a test that fails a check, then skips" STATUS 1
    SAYS "CHECK(1 + 1 == 3) with 2 == 3" "SKIPPED: no device here" "FAILED"
)
_vkm_test_run("a throw fails the test, saying what it threw, and tearDown still runs"
    BINARY vkm_test_fixture TEST "fixture: a test that throws" STATUS 1
    SAYS "test: threw past the end" "torn down fixture: a test that throws" "FAILED"
)
_vkm_test_run("setUp runs before the body, given the test, and tearDown after it"
    BINARY vkm_test_fixture TEST "fixture: a test that passes" STATUS 0
    SAYS "set up fixture: a test that passes" "in the body" "torn down fixture: a test that passes" "ok"
)
_vkm_test_run("a pass exits 0 and leaves no folder behind"
    BINARY vkm_test_fixture TEST "fixture: a test that passes, writing in its folder" STATUS 0
    REMOVED ${_vkm_test_folders}/a_test_that_passes_writing_in_its_folder
)
_vkm_test_run("a failure keeps the test's folder, with what it wrote"
    BINARY vkm_test_fixture TEST "fixture: a test that fails, writing in its folder" STATUS 1
    KEPT ${_vkm_test_folders}/a_test_that_fails_writing_in_its_folder/kept.txt
)
_vkm_test_run("a run named in VKM_TEST_AGAIN has a folder of its own beside the test's"
    BINARY vkm_test_fixture TEST "fixture: a test run again, writing in its folder" STATUS 1
    ENVIRONMENT VKM_TEST_AGAIN=fresh
    KEPT ${_vkm_test_folders}/a_test_run_again_writing_in_its_folder_fresh/kept.txt
)
_vkm_test_run("vkm's tools stay where the user's LOCALAPPDATA keeps them"
    BINARY vkm_test_fixture TEST "fixture: a test that says where vkm's tools are" STATUS 0
    ENVIRONMENT LOCALAPPDATA=${CMAKE_CURRENT_BINARY_DIR}/local VKM_TOOLS_DIR=
    SAYS "VKM_TOOLS_DIR=${CMAKE_CURRENT_BINARY_DIR}/local/vkm/tools"
)
_vkm_test_run("a test's area is the folder under the last tests/ in its path, either slash a separator"
    BINARY vkm_test_fixture TEST --list STATUS 0
    SAYS "net: a test under two tests folders, in the last one's area" "physics: a test whose path takes backslashes"
        "render: a test in a folder below its area"
)
_vkm_test_run("a test no binary has is refused, saying how to list them"
    BINARY vkm_test_fixture TEST "fixture: no such test" STATUS 2
    SAYS "No test called 'fixture: no such test'; --list names them."
)
_vkm_test_run("a suite with two tests of one name, a test with no area, or its hooks set twice does not run, saying each"
    BINARY vkm_test_fixture_broken TEST --list STATUS 2
    SAYS "Two tests are called 'fixture: a name two tests have'."
        "'a test in no folder under tests/' in /nowhere/broken.cpp has no area"
        "The suite sets its hooks 2 times"
)

# Discovery read back through ctest: what it makes of a binary's tests, and of one that cannot answer.
function(_vkm_test_discovery promise case)
    add_test(NAME "vkm_test: ${promise}"
        COMMAND ${CMAKE_COMMAND} -DCASE=${case} -DDISCOVER=${PROJECT_SOURCE_DIR}/cmake/discover.cmake
            -DFIXTURE=$<TARGET_FILE:vkm_test_fixture> -DBROKEN=$<TARGET_FILE:vkm_test_fixture_broken>
            -DOUT=${CMAKE_CURRENT_BINARY_DIR}/discovery/${case} "-DGENERATOR=${CMAKE_GENERATOR}"
            -DMAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} -P ${CMAKE_CURRENT_SOURCE_DIR}/discovery.cmake
    )
    set_tests_properties("vkm_test: ${promise}" PROPERTIES LABELS vkm_test)
endfunction()

_vkm_test_discovery("discovery makes each test a binary lists a ctest of that name, labeled with its area, the suite and LABELS, run in WORKING_DIRECTORY or beside the binary" named)
_vkm_test_discovery("discovery stands one failing ctest for a binary that is not built or cannot list its tests" failing)
# Ninja Multi-Config, which needs the build tool a Ninja build already has.
if(CMAKE_GENERATOR MATCHES "^Ninja")
    _vkm_test_discovery("under a multi-config generator, discovery reads the build type ctest -C names, and fails one ctest when it names none" multi_config)
endif()

# The checks of a library as a consumer builds it, on the fixture libraries: what passes a library in the shape, and what each fails one that is not.
vkm_test_consumer("vkm_test: the consumer check passes a library that stops without a target it requires, naming it" vkm_fixture
    REQUIRES vkm_fixture_needed
)
vkm_test_consumer("vkm_test: the consumer check fails a library that configures without a target it requires" vkm_careless
    REQUIRES vkm_fixture_needed
)
vkm_test_flags("vkm_test: the flags check passes a library whose asserts follow their flag" vkm_fixture ASSERTS ON)
vkm_test_flags("vkm_test: the flags check passes a library whose asserts and profiler follow their flags" vkm_careless
    ASSERTS OFF PROFILER ON
)
vkm_test_flags("vkm_test: the flags check fails a library that keeps its asserts though its flag is off" vkm_fixture ASSERTS OFF)
vkm_test_flags("vkm_test: the flags check fails a library that drops its asserts though its flag is on" vkm_careless ASSERTS ON
    PROFILER ON
)
vkm_test_flags("vkm_test: the flags check fails a library that calls Tracy though its profiler is off" vkm_careless ASSERTS OFF)
# Each check's own failure, by what it says: the check stops with it, which is the test's pass.
set_tests_properties("vkm_test: the consumer check fails a library that configures without a target it requires"
    PROPERTIES PASS_REGULAR_EXPRESSION "without vkm_fixture_needed it configures"
)
set_tests_properties("vkm_test: the flags check fails a library that keeps its asserts though its flag is off"
    PROPERTIES PASS_REGULAR_EXPRESSION "compiles with -UNDEBUG, though its asserts are off"
)
set_tests_properties("vkm_test: the flags check fails a library that drops its asserts though its flag is on"
    PROPERTIES PASS_REGULAR_EXPRESSION "compiles without -UNDEBUG, though its asserts are on"
)
set_tests_properties("vkm_test: the flags check fails a library that calls Tracy though its profiler is off"
    PROPERTIES PASS_REGULAR_EXPRESSION "calls Tracy, though its profiler is off"
)

add_test(NAME "vkm_test: every include is its own or the standard library's, the system's only where a file reaches it"
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}/src -P ${CMAKE_CURRENT_SOURCE_DIR}/includes.cmake
)
set_tests_properties("vkm_test: every include is its own or the standard library's, the system's only where a file reaches it"
    PROPERTIES LABELS vkm_test
)
