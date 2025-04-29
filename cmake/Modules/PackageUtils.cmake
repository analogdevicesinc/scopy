# cmake/PackageUtils.cmake

function(add_subdirectories_with_options base_dir output_dir prefix)
    file(GLOB CHILDREN RELATIVE ${base_dir} ${base_dir}/*)

    foreach(child ${CHILDREN})
        if(IS_DIRECTORY ${base_dir}/${child})
            string(TOUPPER ${child} CHILD_UPPER)
            set(ENABLE_CHILD_VAR "${prefix}_${CHILD_UPPER}")

            # Set default ON
            if(NOT DEFINED ${ENABLE_CHILD_VAR})
                set(${ENABLE_CHILD_VAR} ON CACHE BOOL "Enable ${prefix} ${child}")
            endif()

            if(${ENABLE_CHILD_VAR})
                message(STATUS "Including ${prefix}: ${child}")

                # Set output directories based on platform
                if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${output_dir})
                elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Scopy.app/Contents/MacOS/${output_dir}")
                else()
		    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${output_dir})
                endif()

                add_subdirectory(${base_dir}/${child})
            else()
                message(STATUS "Skipping ${prefix}: ${child}")
            endif()
        endif()
    endforeach()
endfunction()
