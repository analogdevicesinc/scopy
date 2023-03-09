cmake_minimum_required(VERSION 3.5)

function(SETUP_TESTS)
    foreach(_testname ${ARGS})
        add_executable(_testname ${ARGS})
        add_test(NAME ${_testname} COMMAND ${_testname})
	target_link_libraries(${_testname} Qt${QT_VERSION_MAJOR}::Test)
    endforeach()
endfunction()


# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt${QT_VERSION_MAJOR}Test REQUIRED)

SETUP_TESTS(
    tst_test1
)

