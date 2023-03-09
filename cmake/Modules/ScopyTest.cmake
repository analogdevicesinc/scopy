if(DEFINED __INCLUDED_SCOPY_STYLESHEETS_CMAKE)
    return()
endif()
set(__INCLUDED_SCOPY_STYLESHEETS_CMAKE TRUE)



# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Test REQUIRED)


function(SETUP_SCOPY_TESTS)
    foreach(_testname ${ARGN})
        set(_test_target ${PROJECT_NAME}_test_${_testname})
        add_executable(${_test_target} tst_${_testname}.cpp)
        add_test(NAME ${_test_target} COMMAND ${_test_target})
	target_link_libraries(${_test_target} Qt5::Test ${PROJECT_NAME})
    endforeach()
endfunction()
