if(DEFINED __INCLUDED_SCOPY_TEST_CMAKE)
    return()
endif()
set(__INCLUDED_SCOPY_TEST_CMAKE TRUE)



# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)


function(configure_stylesheets DU_OPTION)
        file(GLOB_RECURSE STYLESHEETS ${CMAKE_CURRENT_SOURCE_DIR}/resources/stylesheets/templates/*.qss.c)

        foreach(_stylesheet ${STYLESHEETS})
                string(REPLACE ".c" "" FILE_OUT ${_stylesheet})
                string(REPLACE "templates/" "" FILE_OUT ${FILE_OUT})

                execute_process (
                        COMMAND ${CMAKE_C_COMPILER} -E -P ${DU_OPTION} ${_stylesheet} -o ${FILE_OUT}
                        )

                message(STATUS "Done preprocessing ${_stylesheet}, file written to: ${FILE_OUT}")
        endforeach()
endfunction()
