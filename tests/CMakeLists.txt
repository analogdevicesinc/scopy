cmake_minimum_required(VERSION 3.5)

# JS AUTOMATED TESTS
option(ENABLE_AUTOMATED_TESTS "Enable JS Automated Tests" OFF)

if(ENABLE_AUTOMATED_TESTS)
	# Set build path as CMAKE CACHE variable to use in tests
	set(SCOPY_BUILD_PATH ${CMAKE_BINARY_DIR} CACHE STRING "SCOPY_BUILD_PATH" FORCE)

	# SCOPY GENERIC TESTS
	add_test(NAME "ScopyDefaultJSTests"
		 COMMAND bash ${CMAKE_SOURCE_DIR} / js / test.sh "${CMAKE_SOURCE_DIR}/resources/emuXml/"
			 "${CMAKE_SOURCE_DIR}/js/scopyDefaultTests.js"
	)

	# PLUGIN SPECIFIC TESTS

	# DATALOGGER
	if(ENABLE_PLUGIN_DATALOGGER)
		add_test(NAME "DataloggerLogAndLoadJSTests"
			 COMMAND bash ${CMAKE_SOURCE_DIR} / js / test.sh "${CMAKE_SOURCE_DIR}/resources/emuXml/"
				 "${CMAKE_SOURCE_DIR}/plugins/datalogger/js/dataLoggerAutomatedTest.js"
		)
	endif()

endif()
