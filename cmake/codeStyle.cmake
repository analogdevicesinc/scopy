option(ENABLE_CMAKE_FORMAT "Enable code style checking with cmake-format" OFF)

if(ENABLE_CMAKE_FORMAT)
    find_program(CMAKEFORMAT cmake-format)
    if(CMAKEFORMAT)
        file(GLOB_RECURSE CMAKE_FILES *.cmake CMakeLists.txt)

        # ignore old source code files
        foreach(element ${CMAKE_FILES})
            string(REGEX MATCH ".*old.*" result ${element})
            if(result)
                list(REMOVE_ITEM CMAKE_FILES ${element})
            endif()
        endforeach()

        add_custom_target(
            cmake-format COMMAND ${CMAKEFORMAT} -i ${CMAKE_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    else()
        message(SEND_ERROR "cmake-format requested but executable not found")
    endif()
endif()
