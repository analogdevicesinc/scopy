option(ENABLE_CPPCHECK "Enable static analysis with cppcheck" OFF)
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
option(ENABLE_INCLUDE_WHAT_YOU_USE
       "Enable static analysis with include-what-you-use" OFF
)
option(ENABLE_SHELLCHECK "Enable static analysis with shellcheck" OFF)

if(ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
        set(CMAKE_CXX_CPPCHECK
            ${CPPCHECK}
            --suppress=missingInclude
            --enable=all
            --inline-suppr
            --inconclusive
            -i
            ${CMAKE_SOURCE_DIR}/imgui/lib
        )
    else()
        message(SEND_ERROR "cppcheck requested but executable not found")
    endif()
endif()

if(ENABLE_CLANG_TIDY)
    find_program(CLANGTIDY clang-tidy)
    if(CLANGTIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY}
                                 -extra-arg=-Wno-unknown-warning-option
        )
    else()
        message(SEND_ERROR "clang-tidy requested but executable not found")
    endif()
endif()

if(ENABLE_INCLUDE_WHAT_YOU_USE)
    find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
    if(INCLUDE_WHAT_YOU_USE)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
    else()
        message(
            SEND_ERROR "include-what-you-use requested but executable not found"
        )
    endif()
endif()

if(ENABLE_SHELLCHECK)
    find_program(SHELLCHECK shellcheck)
    if(SHELLCHECK)
        file(GLOB_RECURSE SH_FILES *.sh)

        # ignore old source code files
        foreach(element ${SH_FILES})
            string(REGEX MATCH ".*old.*" result ${element})
            if(result)
                list(REMOVE_ITEM SH_FILES ${element})
            endif()
        endforeach()

        add_custom_target(
            shellcheck COMMAND ${SHELLCHECK} ${SH_FILES} || (exit 0)
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    else()
        message(SEND_ERROR "shellcheck requested but executable not found")
    endif()
endif()
