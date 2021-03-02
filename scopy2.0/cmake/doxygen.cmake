option(ENABLE_DOXYGEN "Enable documentation generation with Doxygen" OFF)

# enable doxygen documentation on the given targets. The files that are
# documented are obtained from the INTERFACE_INCLUDE_DIRS of the targets as this
# property is populated with the headers that are exported (public API)
function(enable_doxygen targets)
    if(ENABLE_DOXYGEN)
        find_package(Doxygen)

        set(HEADERS "")
        foreach(target ${targets})
            get_target_property(
                include_headers ${target} INTERFACE_INCLUDE_DIRECTORIES
            )
            string(REPLACE "$<INSTALL_INTERFACE:include>" "" include_headers
                           ${include_headers}
            )
            string(REPLACE "$<BUILD_INTERFACE:" "" include_headers
                           ${include_headers}
            )
            string(REPLACE ">" "" include_headers ${include_headers})
            set(HEADERS "${HEADERS}\;${include_headers}")
        endforeach()

        doxygen_add_docs(doxygen ${HEADERS} COMMENT "Generating doxygen doc")
    endif()
endfunction()
