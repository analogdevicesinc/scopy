if(DEFINED __INCLUDED_SCOPY_WINDOWS_CMAKE)
	return()
endif()
set(__INCLUDED_SCOPY_WINDOWS_CMAKE TRUE)

# Env variables
set(ENV_WORKDIR $ENV{WORKDIR})
set(ENV_DEST_FOLDER $ENV{DEST_FOLDER})

configure_file(windows/scopy-64.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/windows/scopy-64.iss @ONLY)

macro(duplicate_target ORIGINAL NEW)
	get_target_property(original_libraries ${ORIGINAL} LINK_LIBRARIES)
	get_target_property(original_includes ${ORIGINAL} INCLUDE_DIRECTORIES)
	get_target_property(original_options ${ORIGINAL} COMPILE_OPTIONS)

	add_executable(
		${NEW} WIN32 ${PROJECT_SOURCES} ${SCOPY_RESOURCES} ${SCOPY_DEPENDENCIES} ${SCOPY_WIN32_RESOURCES}
	)

	target_link_libraries(${NEW} PRIVATE ${original_libraries})
	target_include_directories(${NEW} PRIVATE ${original_includes})
	target_compile_options(${NEW} PRIVATE ${original_options})
endmacro()
