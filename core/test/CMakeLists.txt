cmake_minimum_required(VERSION 3.5)

message("Testing123")

function(SETUP_TESTS)
    message("SETUP_TESTS " ${ARGN})
    foreach(_testname ${ARGN})
        add_executable(${_testname} tst_${_testname}.cpp)
        add_test(NAME ${_testname} COMMAND ${_testname})
        message("testname: " ${_testname} " " tst_${_testname}.cpp)
        target_link_libraries(${_testname} Qt5::Test scopycore)
    endforeach()
endfunction()


# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Test REQUIRED)

SETUP_TESTS(
    mainwindow
)

