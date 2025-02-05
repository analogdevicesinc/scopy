#!/bin/python3

import sys

def update_cmake_file(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        lines = f.readlines()
    
    modified = False
    
    # Search rules
    include_dirs_line = "target_include_directories(${PROJECT_NAME} PRIVATE ${PDK_DEPS_INCLUDE})\n"
    pdk_support_line = "include(${CMAKE_SOURCE_DIR}/PdkSupport.cmake)\n"
    include_dirs_pdk = "include_dirs(${PDK_DEPS_INCLUDE})\n"
    link_libs_pdk = "link_libs(${PDK_DEPS_LIB})\n"
    
    # Search for the last target_include_directories
    last_include_index = max((i for i, line in enumerate(lines) if "target_include_directories" in line), default=-1)
    
    # Include dirs line if not exists
    if not any(include_dirs_line.strip() in line for line in lines):
        lines.insert(last_include_index + 1, include_dirs_line)
        modified = True
    
    # Include PdkSupport.cmake if not exists
    if not any(pdk_support_line.strip() in line for line in lines):
        lines.insert(last_include_index + 2, pdk_support_line)
        modified = True
    
    # Include include_dirs instruction
    if not any(include_dirs_pdk.strip() in line for line in lines):
        lines.insert(last_include_index + 3, include_dirs_pdk)
        modified = True
    
    # Search for the last target_link_libraries
    last_link_index = max((i for i, line in enumerate(lines) if "target_link_libraries" in line), default=-1)
    
    # Include link_libs instruction
    if not any(link_libs_pdk.strip() in line for line in lines):
        lines.insert(last_link_index + 1, link_libs_pdk)
        modified = True
    
    # Apply the changes
    if modified:
        with open(file_path, "w", encoding="utf-8") as f:
            f.writelines(lines)
        print("The file is updated!")
    else:
        print("No update is required!")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Run: update_cmake.py <path_to_CMakeLists.txt>")
        sys.exit(1)
    
    update_cmake_file(sys.argv[1])
