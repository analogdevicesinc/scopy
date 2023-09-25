if(DEFINED __INCLUDED_SCOPY_TRANSLATIONS_CMAKE)
	return()
endif()
set(__INCLUDED_SCOPY_TRANSLATIONS_CMAKE TRUE)

function(generate_translations)
	file(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/translations/*.ts)
	set_source_files_properties(
		${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_BINARY_DIR}/translations
				       MACOSX_PACKAGE_LOCATION ${CMAKE_BINARY_DIR}/translations
	)
	qt_add_translation(QM_FILES ${TS_FILES})

	set(TRANSLATIONS)
	foreach(file ${TS_FILES})
		get_filename_component(file_name ${file} NAME_WE)
		set(TRANSLATIONS "${TRANSLATIONS}\n<file>${CMAKE_BINARY_DIR}/translations/${file_name}.qm</file>")
	endforeach()

	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/resources/translations.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
		@ONLY
	)
endfunction()
