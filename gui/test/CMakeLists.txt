cmake_minimum_required(VERSION 3.5)

message("Testing!!")

function(SETUP_TESTS)
    foreach(_testname ${ARGS})
        add_executable(_testname ${ARGS})
        add_test(NAME ${_testname} COMMAND ${_testname})
        target_link_libraries(${_testname} Qt5::Test QWAMLib)
    endforeach()
endfunction()


# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Test REQUIRED)

SETUP_TESTS(
    tst_test1
)

