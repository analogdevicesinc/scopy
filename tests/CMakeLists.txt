cmake_minimum_required(VERSION 3.5)

include(ScopyTest)

# JS AUTOMATED TESTS

# Set build path as CMAKE CACHE variable to use in tests
set(SCOPY_BUILD_PATH ${CMAKE_BINARY_DIR} CACHE STRING "SCOPY_BUILD_PATH" FORCE)
set(SCOPY_SOURCE_PATH ${CMAKE_SOURCE_DIR} CACHE STRING "SCOPY_SOURCE_PATH" FORCE)


#SCOPY BASE TESTS
#add_test(NAME "DeviceConnectionJSTest" COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/automatedJSTests/scopyDeviceConnectionTest.sh)
add_test(NAME "SwitchToolJSTest" COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/automatedJSTests/scopySwitchToolTest.sh)


# PLUGIN SPECIFIC TESTS

# DATALOGGER SPECIFIC TESTS
add_test(NAME "DataLoggerJSTest" COMMAND bash ${CMAKE_SOURCE_DIR}/plugins/datalogger/test/dataLoggerAtuomatedTest.sh)

