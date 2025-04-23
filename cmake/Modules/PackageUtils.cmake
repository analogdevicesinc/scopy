# cmake/PackageUtils.cmake

function(add_all_packages PACKAGES_DIR)
    file(GLOB CHILD_DIRS RELATIVE ${PACKAGES_DIR} ${PACKAGES_DIR}/*)

    foreach(PACKAGE_NAME ${CHILD_DIRS})
	set(PACKAGE_PATH "${PACKAGES_DIR}/${PACKAGE_NAME}")

	if(IS_DIRECTORY "${PACKAGE_PATH}" AND EXISTS "${PACKAGE_PATH}/CMakeLists.txt")
	    string(TOUPPER ${PACKAGE_NAME} PACKAGE_UPPER)
	    set(ENABLE_VAR "ENABLE_PACKAGE_${PACKAGE_UPPER}")

	    # Set default ON
	    if(NOT DEFINED ${ENABLE_VAR})
		set(${ENABLE_VAR} ON CACHE BOOL "Enable package ${PACKAGE_NAME}")
	    endif()

	    if(${ENABLE_VAR})
		message(STATUS "Including package: ${PACKAGE_NAME}")
		add_subdirectory(${PACKAGE_PATH})
	    else()
		message(STATUS "Skipping package: ${PACKAGE_NAME}")
	    endif()
	endif()
    endforeach()
endfunction()
