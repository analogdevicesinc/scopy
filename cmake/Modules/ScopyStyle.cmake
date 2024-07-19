if(DEFINED __INCLUDED_SCOPY_STYLE_CMAKE)
	return()
endif()
set(__INCLUDED_SCOPY_STYLE_CMAKE TRUE)

# option can be --core or --plugin
function(generate_style option style_folder headers_folder)
	find_package(Python3 COMPONENTS Interpreter)
	execute_process(
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/style_generator.py ${option} ${style_folder}
			${headers_folder} ${CMAKE_BINARY_DIR} RESULT_VARIABLE ret
	)
	if(NOT ret EQUAL "0")
		message(FATAL_ERROR "Failed to generate style files! error: ${ret}")
	else()
		message("-- Generated style files")
	endif()
endfunction()
