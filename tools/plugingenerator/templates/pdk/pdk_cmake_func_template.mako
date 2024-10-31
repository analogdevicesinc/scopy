# Define a function to include all directories recursively
function(inlcude_dirs root_dir)
	# Find all files and directories recursively
	file(GLOB_RECURSE all_items LIST_DIRECTORIES true ""${"${root_dir}"}/"")
	# Loop through each item found
	foreach(item ${"${all_items}"})
		# Check if the item is a directory
		if(IS_DIRECTORY ${"${item}"})
			message(${"${item}"})
			target_include_directories(${"${PROJECT_NAME}"} PRIVATE ${"${item}"})
		endif()
	endforeach()
endfunction()

# Define a function to link all .so files from a root directory
function(link_libs root_dir)
	# Find all .so files from root_dir
	file(GLOB all_libs "${"${root_dir}"}/*.so")
	# Loop through each library found
	foreach(lib ${"${all_libs}"})
		# Link libraries
		message(${"${lib}"})
		target_link_libraries(${"${PROJECT_NAME}"} PRIVATE ${"${lib}"})
	endforeach()
endfunction()