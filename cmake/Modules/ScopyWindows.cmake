if(DEFINED __INCLUDED_SCOPY_WINDOWS_CMAKE)
    return()
endif()
set(__INCLUDED_SCOPY_WINDOWS_CMAKE TRUE)

configure_file(windows/scopy.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/windows/scopy.iss @ONLY)
configure_file(windows/scopy-32.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/windows/scopy-32.iss @ONLY)
configure_file(windows/scopy-64.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/windows/scopy-64.iss @ONLY)
