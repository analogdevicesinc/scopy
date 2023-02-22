cmake_minimum_required(VERSION 3.5)

message("Testing: " ${PROJECT_NAME})

function(SETUP_TESTS)
    message("SETUP_TESTS " ${ARGN})
    foreach(_testname ${ARGN})
        set(_test_target ${PROJECT_NAME}_${_testname})
        add_executable(${_test_target} tst_${_testname}.cpp)
        add_test(NAME ${_test_target} COMMAND ${_test_target})
        message("testname: " ${_test_target} " " tst_${_testname}.cpp)
        target_link_libraries(${_test_target} Qt5::Test ${PROJECT_NAME})
    endforeach()
endfunction()


# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Test REQUIRED)

SETUP_TESTS(
    pluginloader
)

