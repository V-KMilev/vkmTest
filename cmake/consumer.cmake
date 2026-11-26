# A library added to a project of its own (consumer/) as a consumer adds it, once with every target it requires added first and once without each of them: with all of them it configures, and without one it stops and names it. Included, this file defines vkm_test_consumer(<name> <library> [REQUIRES <target>...]), which adds that check of the target <library> as the ctest <name>, labeled with the prefix before its colon; ctest runs this file again with -P.

if(NOT CMAKE_SCRIPT_MODE_FILE)
    function(vkm_test_consumer name library)
        cmake_parse_arguments(PARSE_ARGV 2 _arg "" "" "REQUIRES")
        if(_arg_UNPARSED_ARGUMENTS)
            message(FATAL_ERROR "vkm_test_consumer(\"${name}\" ${library}) takes REQUIRES, not: ${_arg_UNPARSED_ARGUMENTS}")
        endif()
        get_target_property(_library_dir ${library} SOURCE_DIR)
        string(REPLACE ";" "|" _requires "${_arg_REQUIRES}")
        string(SHA1 _folder "${name}")
        string(SUBSTRING ${_folder} 0 8 _folder)
        add_test(NAME ${name}
            COMMAND ${CMAKE_COMMAND} -DLIBRARY_DIR=${_library_dir} -DREQUIRES=${_requires}
                -DOUT=${CMAKE_CURRENT_BINARY_DIR}/consumer/${_folder} -DGENERATOR=${CMAKE_GENERATOR}
                -P ${CMAKE_CURRENT_FUNCTION_LIST_FILE}
        )
        string(REGEX MATCH "^[^:]+" _label "${name}")
        set_tests_properties(${name} PROPERTIES LABELS ${_label})
    endfunction()
    return()
endif()

string(REPLACE "|" ";" _requires "${REQUIRES}")
file(REMOVE_RECURSE "${OUT}")

set(_problems "")
foreach(_missing "" ${_requires})
    set(_adds ${_requires})
    if(_missing)
        list(REMOVE_ITEM _adds ${_missing})
    endif()
    string(REPLACE ";" "|" _adds "${_adds}")
    set(_case "without ${_missing}")
    if(NOT _missing)
        set(_case "with all")
    endif()

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -S "${CMAKE_CURRENT_LIST_DIR}/consumer" -B "${OUT}/${_case}" -G "${GENERATOR}"
                "-DVKM_LIBRARY_DIR=${LIBRARY_DIR}" "-DVKM_CONSUMER_ADDS=${_adds}"
        RESULT_VARIABLE _status
        OUTPUT_VARIABLE _output
        ERROR_VARIABLE  _output
    )
    if(NOT _missing AND NOT _status EQUAL 0)
        string(APPEND _problems "  with ${REQUIRES} added first it does not configure:\n${_output}\n")
    elseif(_missing AND _status EQUAL 0)
        string(APPEND _problems "  without ${_missing} it configures\n")
    elseif(_missing AND NOT _output MATCHES "requires the target '${_missing}'")
        string(APPEND _problems "  without ${_missing} it stops without naming it:\n${_output}\n")
    endif()
endforeach()

if(_problems)
    message(FATAL_ERROR "As a consumer's subdirectory:\n${_problems}")
endif()
file(REMOVE_RECURSE "${OUT}")
