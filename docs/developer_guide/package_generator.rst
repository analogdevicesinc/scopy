.. _package_generator:

Scopy Package Generator Guide 
===========================================================

This guide provides instructions on how to use the Scopy package generator, including the plugin generator.

Pre-requisites
----------------

Ensure you have the necessary dependencies installed:

```
pip install -r requirements.txt
```

Config.json Attributes Explanation
----------------------------------------

The `config.json` file contains various attributes that define the plugin configuration. Here is an explanation of each attribute:

1. **pdk**: Contains the configuration options of the plugin development tool.
    - `enable` (bool): If true, a ScopyPluginRunner project will be created at `project_path`.
    - `deps_path` (string): The path to the Scopy PDK package (the package must be unzipped).
    - `project_path` (string): The path to the directory where the PluginRunner project is created.
2. **plugin**: Contains details specific to the plugin.
    - `dir_name` (string): The name of the plugin directory.
    - `plugin_name` (string): The plugin name. The header and source files will have this name.
    - `class_name` (string): The name of the class that will be created for the plugin.
    - `cmakelists` (bool): If true, a CMakeLists file will be created for the plugin.
    - `namespace` (string): The namespace the plugin belongs to.
    - `device_category` (string): The category the plugin belongs to (e.g., iio).
    - `tools`: List which contains the tools to be implemented and used by the plugin.
        - `id` (string): Tool id.
        - `tool_name` (string): The name that appears in Scopy.
        - `file_name` (string): The tool header and source file will have this name.
        - `class_name` (string): The name of the class that will be created for the tool.
        - `namespace` (string): The namespace the tool belongs to.
3. **cmakelists**: Contains the details of the CMakeLists.
    - `cmake_min_required` (float): Require a minimum version of CMake.
    - `cxx_standard` (int): The C++ standard whose features are requested to build this target.
    - `enable_testing` (string): Enables the tests for the plugin. The only accepted variants are "ON" and "OFF".
4. **test**: Contains the details of the plugin tests.
    - `mkdir` (bool): If true, the test directory is created.
    - `cmakelists` (bool): If true, a CMakeLists file will be created for the tests.
    - `cmake_min_required` (float): Require a minimum version of CMake.
    - `tst_pluginloader` (bool): If true, the pluginloader test is created.
5. **doc**: Contains the details of the plugin documentation.
    - `mkdir` (bool): If true, the doc directory is created.
6. **resources**: Contains the details of the plugin resources.
    - `mkdir` (bool): If true, the res directory is created.
    - `resources_qrc` (bool): If true, the resources_qrc file is created.

**All fields must be filled in**


Running the Plugin Generator Script
-------------------------------------

The plugin generator script can be run in two ways:

1. If it is run from "scopy/tools/plugingenerator":

    ```
    ./plugin_generator.py
    ```

2. Otherwise:

    ```
    python plugin_generator.py --scopy_path=path/to/scopy --config_file_path=path/to/config/file.json
    ```

For help, use:

```
python plugin_generator.py -h
```

After running the script, you must insert the new plugin in plugins/CMakeLists.txt
------------------------------------------------------------------------------------

Add the following lines to `plugins/CMakeLists.txt`:

```
option(ENABLE_PLUGIN_NEWPLUGIN "Enable NEWPLUGIN plugin" ON)
if(ENABLE_PLUGIN_NEWPLUGIN)
add_subdirectory(new)
list(APPEND PLUGINS ${PLUGIN_NAME})
endif()
```

Scopy PDK
===========

Add New Modules to the PDK Package
--------------------------------------

To add a new module from Scopy to the package, ensure that when installing libraries or directories in CMakeLists, the option `COMPONENT ${SCOPY_PDK}` is added.

- Libraries: `install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SCOPY_DLL_INSTALL_PATH} COMPONENT ${SCOPY_PDK} RUNTIME DESTINATION ${SCOPY_DLL_INSTALL_PATH})`.
- Headers: `install(DIRECTORY include/ DESTINATION include/ COMPONENT ${SCOPY_PDK})`.
- Resources: `install(DIRECTORY res/ DESTINATION resources/ COMPONENT ${SCOPY_PDK})`.

Develop a New Plugin Using ScopyPluginRunner
----------------------------------------------

1. Download the PDK package from the Scopy repository.
    - If the package is not available or if changes have been made to the Scopy libraries, then Scopy must be built locally with the `--target package` option.
        - In this case, a `package` folder will be generated inside the Scopy build folder.
2. Generate the ScopyPluginRunner project using the `plugin_generator.py`.
    - Fill in the `config.json` file.
    - This project contains a `plugin` submodule where the desired plugin can be developed.
3. Choose an IDE and start working.
    - Our recommendation is to use QtCreator.

Adding New Config Specifications
-----------------------------------

For fields like `plugin`, `tool`, and `cmakelists`, new specifications can be added. Follow these steps:

1. Add the new specification in the desired field (add new fields in `config.json`).
2. Use the specification in the templates. Each template contains a `config` variable through which the information from each field can be accessed. A specific syntax must be used because the script uses the mako library to render the templates. [Watch mako documentation page for more details.](https://docs.makotemplates.org/en/latest/)
    - Changes made in the `plugin` field will be reflected in the **plugin_src_template** and **plugin_header_template** template files.
    - Changes made in the `tool` field will be reflected in the **tool_src_template** and **tool_header_template** template files.
    - Changes made in the `cmakelists` field will be reflected in the **cmakelists_template** template file.
3. Run the `plugin_generator.py` script.
