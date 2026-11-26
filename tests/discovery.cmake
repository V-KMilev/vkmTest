# vkm_discover_tests, read back from ctest (cmake -P): fixture/discovery/ discovers a binary's tests, configured once for each case, and what `ctest -N --show-only=json-v1` says of them is checked against what the binary lists.
#
#   -DCASE=<case>            named, failing or multi_config
#   -DDISCOVER=<file>        cmake/discover.cmake
#   -DFIXTURE=<file>         a binary whose tests are in several areas
#   -DBROKEN=<file>          a binary that cannot list its tests
#   -DOUT=<folder>           where the projects are configured
#   -DGENERATOR=<generator>  and with which generator, Ninja Multi-Config for multi_config
#   -DMAKE_PROGRAM=<file>    the generator's build tool

set(_problems "")

# The project configured as <case> in OUT, discovering <binary> with vkm_discover_tests' further arguments, and read back as ctest -N reads it with ctest's arguments after the '--'.
function(_tests_of case binary arguments)
    cmake_parse_arguments(PARSE_ARGV 3 _arg "" "GENERATOR" "CTEST")
    if(NOT _arg_GENERATOR)
        set(_arg_GENERATOR "${GENERATOR}")
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -S "${CMAKE_CURRENT_LIST_DIR}/fixture/discovery" -B "${OUT}/${case}"
                -G "${_arg_GENERATOR}" "-DCMAKE_MAKE_PROGRAM=${MAKE_PROGRAM}" "-DVKM_DISCOVER=${DISCOVER}" "-DVKM_BINARY=${binary}"
                "-DVKM_ARGUMENTS=${arguments}"
        RESULT_VARIABLE _status
        OUTPUT_VARIABLE _output
        ERROR_VARIABLE  _output
    )
    if(NOT _status EQUAL 0)
        message(FATAL_ERROR "${case} does not configure:\n${_output}")
    endif()
    execute_process(
        COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${OUT}/${case}" -N --show-only=json-v1 ${_arg_CTEST}
        RESULT_VARIABLE _status
        OUTPUT_VARIABLE _json
        ERROR_VARIABLE  _errors
    )
    if(NOT _status EQUAL 0)
        message(FATAL_ERROR "ctest cannot list ${case}:\n${_errors}")
    endif()
    set(_json "${_json}" PARENT_SCOPE)
endfunction()

# What test <index> of _json holds: _name, _command, and _<PROPERTY> for each of its properties.
macro(_test_at index)
    string(JSON _name GET "${_json}" tests ${index} name)
    string(JSON _length LENGTH "${_json}" tests ${index} command)
    set(_command "")
    math(EXPR _last "${_length} - 1")
    foreach(_part RANGE ${_last})
        string(JSON _word GET "${_json}" tests ${index} command ${_part})
        list(APPEND _command "${_word}")
    endforeach()
    foreach(_property LABELS WILL_FAIL SKIP_RETURN_CODE WORKING_DIRECTORY)
        set(_${_property} "")
    endforeach()
    string(JSON _length LENGTH "${_json}" tests ${index} properties)
    math(EXPR _last "${_length} - 1")
    foreach(_at RANGE ${_last})
        string(JSON _property GET "${_json}" tests ${index} properties ${_at} name)
        string(JSON _type TYPE "${_json}" tests ${index} properties ${_at} value)
        if(_type STREQUAL "ARRAY")
            set(_${_property} "")
            string(JSON _count LENGTH "${_json}" tests ${index} properties ${_at} value)
            math(EXPR _end "${_count} - 1")
            foreach(_item RANGE ${_end})
                string(JSON _word GET "${_json}" tests ${index} properties ${_at} value ${_item})
                list(APPEND _${_property} "${_word}")
            endforeach()
        else()
            string(JSON _${_property} GET "${_json}" tests ${index} properties ${_at} value)
        endif()
    endforeach()
endmacro()

# Each test the binary lists is one ctest of the same name, running that test, labeled with its area, the suite and <labels>, in <folder>, skipped on 77; and nothing else is.
macro(_expect_listed case labels folder)
    execute_process(COMMAND "${FIXTURE}" --list OUTPUT_VARIABLE _listed RESULT_VARIABLE _status)
    string(STRIP "${_listed}" _listed)
    string(REPLACE "\n" ";" _listed "${_listed}")
    string(JSON _count LENGTH "${_json}" tests)
    list(LENGTH _listed _expected)
    if(NOT _count EQUAL _expected)
        string(APPEND _problems "  ${case}: ${_count} ctests for the ${_expected} tests the binary lists\n")
    endif()
    math(EXPR _last "${_count} - 1")
    foreach(_index RANGE ${_last})
        _test_at(${_index})
        list(FIND _listed "${_name}" _found)
        string(REGEX MATCH "^[^:]+" _area "${_name}")
        # ctest sorts a test's labels.
        set(_labels ${_area} vkm_fixture ${labels})
        list(SORT _labels)
        if(_found EQUAL -1)
            string(APPEND _problems "  ${case}: '${_name}' is no test the binary lists\n")
        elseif(NOT _command STREQUAL "${FIXTURE};${_name}")
            string(APPEND _problems "  ${case}: '${_name}' runs ${_command}\n")
        elseif(NOT _LABELS STREQUAL "${_labels}")
            string(APPEND _problems "  ${case}: '${_name}' is labeled ${_LABELS}, not ${_labels}\n")
        elseif(NOT _WORKING_DIRECTORY STREQUAL "${folder}")
            string(APPEND _problems "  ${case}: '${_name}' runs in ${_WORKING_DIRECTORY}, not ${folder}\n")
        elseif(NOT _SKIP_RETURN_CODE EQUAL 77)
            string(APPEND _problems "  ${case}: '${_name}' is skipped on '${_SKIP_RETURN_CODE}', not 77\n")
        endif()
    endforeach()
endmacro()

# The one ctest a binary that cannot answer stands as: <name>, labeled vkm_fixture and expected to fail when <fails>.
macro(_expect_one case name fails)
    string(JSON _count LENGTH "${_json}" tests)
    if(NOT _count EQUAL 1)
        string(APPEND _problems "  ${case}: ${_count} ctests, not one\n")
    else()
        _test_at(0)
        if(NOT _name STREQUAL "${name}")
            string(APPEND _problems "  ${case}: the one ctest is '${_name}', not '${name}'\n")
        elseif(NOT _LABELS STREQUAL "vkm_fixture")
            string(APPEND _problems "  ${case}: '${_name}' is labeled ${_LABELS}\n")
        elseif(${fails} AND NOT _WILL_FAIL)
            string(APPEND _problems "  ${case}: '${_name}' is not expected to fail\n")
        endif()
    endif()
endmacro()

file(REMOVE_RECURSE "${OUT}")
get_filename_component(_beside "${FIXTURE}" DIRECTORY)

if(CASE STREQUAL "named")
    _tests_of(labeled "${FIXTURE}" "LABELS|gpu|WORKING_DIRECTORY|${OUT}/elsewhere")
    _expect_listed(labeled gpu "${OUT}/elsewhere")
    _tests_of(beside "${FIXTURE}" "")
    _expect_listed(beside "" "${_beside}")
elseif(CASE STREQUAL "failing")
    _tests_of(unbuilt "${OUT}/nothing" "")
    _expect_one(unbuilt "vkm_fixture: suite is built" TRUE)
    _tests_of(unlisted "${BROKEN}" "")
    _expect_one(unlisted "vkm_fixture: suite lists its tests" FALSE)
    if(NOT _command STREQUAL "${BROKEN};--list")
        string(APPEND _problems "  unlisted: its one ctest runs ${_command}, not ${BROKEN} --list\n")
    endif()
elseif(CASE STREQUAL "multi_config")
    _tests_of(debug "${FIXTURE}" "" GENERATOR "Ninja Multi-Config" CTEST -C Debug)
    _expect_listed(debug "" "${_beside}")
    _tests_of(no_type "${FIXTURE}" "" GENERATOR "Ninja Multi-Config")
    _expect_one(no_type "vkm_fixture: ctest -C names a build type suite has" TRUE)
else()
    message(FATAL_ERROR "No case called '${CASE}'")
endif()

if(_problems)
    message(FATAL_ERROR "Discovery is not as documented:\n${_problems}")
endif()
file(REMOVE_RECURSE "${OUT}")
