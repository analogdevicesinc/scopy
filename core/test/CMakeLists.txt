cmake_minimum_required(VERSION 3.5)

include(ScopyTest)

setup_scopy_tests(pluginmanager pluginrepository)

# test_translationsrepository
if(ENABLE_TRANSLATION)
	set(TEST_GENERATETRANSLATIONS ${PROJECT_NAME}_test_generatetranslations)
	generate_translations()
	qt_add_resources(TEST_TRANSLATIONS_REPOSITORY "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc")

	if(ANDROID)
		add_library(${TEST_GENERATETRANSLATIONS} SHARED ${TEST_TRANSLATIONS_REPOSITORY})
	else()
		add_library(${TEST_GENERATETRANSLATIONS} ${TEST_TRANSLATIONS_REPOSITORY})
	endif()

	# move test translation files to core/tests
	add_custom_command(
		TARGET ${TEST_GENERATETRANSLATIONS} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/translations/test.qm
			${CMAKE_CURRENT_BINARY_DIR}/translations/test.qm
	)
	add_custom_command(
		TARGET ${TEST_GENERATETRANSLATIONS} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove
								       ${CMAKE_BINARY_DIR}/translations/test.qm
	)
	add_custom_command(
		TARGET ${TEST_GENERATETRANSLATIONS} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/translations/test_test.qm
			${CMAKE_CURRENT_BINARY_DIR}/translations/test_test.qm
	)
	add_custom_command(
		TARGET ${TEST_GENERATETRANSLATIONS} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove
								       ${CMAKE_BINARY_DIR}/translations/test_test.qm
	)

	setup_scopy_tests(translationsrepository)
endif()
