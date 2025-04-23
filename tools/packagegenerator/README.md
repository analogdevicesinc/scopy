# Plugin generator

## Pre-request

`pip install -r requirements.txt`

## Run plugin generator script

1. If it is run from "scopy/tools/plugingenerator"

    `./plugin_generator.py`

2. Otherwise

    `python plugin_generator.py --scopy_path=path/to/scopy --config_file_path=path/to/config/file.json`

For help: `python plugin_generator.py -h`  

### After running the script, you must insert the new plugin in plugins/CMakeLists.txt

```
option(ENABLE_PLUGIN_NEWPLUGIN "Enable NEWPLUGIN plugin" ON)
if(ENABLE_PLUGIN_NEWPLUGIN)
 add_subdirectory(new)
 list(APPEND PLUGINS ${PLUGIN_NAME})
endif()
```

## Config.json attributes explanation

1. **pdk**: Contains the configuration options of the plugin development tool.
    1. `enable` (bool): If true a ScopyPluginRunner project will be created at `project_path`.
        - If pdk is enabled, the plugin will be generated in the ScopyPluginRunner project.
    2. `deps_path` (string): The path to the scopy pdk package (the package must be unzipped).
    3. `project_path` (string): The path to the directory where the PluginRunner project is created.
2. **plugin**: Contains details specific to the plugin.
    1. `dir_name` (string): The name of the plugin directory.
    2. `plugin_name` (string): The plugin name. The header and source files will have this name.
    3. `class_name` (string): The name of the class that will be created for the plugin.
    4. `cmakelists` (bool): If true a CMakeLists file will be created for the plugin.
    5. `namespace` (string): The namespace the plugin belongs to.
        - The name of namespace should be short, meaningful, concise, and all lower-case.
        - Try to avoid use of underscores inside namespace names.
        - The keyword "namespace" is filled in automatically. All you have to do is to write the identifier (name of the namespace).
        - For nested namespaces use the scope resolution operator (::).
    6. `device_category` (string): The category the plugin belongs to. (iio for example)
    7. `tools`: List which contains the tools to be implemented and used by the plugin.
        1. `id` (string): Tool id.
        2. `tool_name` (string): The name that appears in Scopy.
        3. `file_name` (string): The tool header and source file will have this name.
            - No special character is allowed in the file name except for underscore (‘_’) and dash (‘-‘).
            - Do not use filenames that already exist in /user/include. or any predefined header file name.
        4. `class_name` (string): The name of the class that will be created for the tool.
            - The class name should be a noun.
            - Use upper case letters as word separators, and lower case for the rest of the word.
            - The first character in the class name must be in upper case.
            - No underscores (‘_’) are permitted in the class name.
        5. `namespace` (string): The namespace the tool belongs to.
3. **cmakelists**: Contains the details of the CMakeLists.
    1. `cmake_min_required` (float): Require a minimum version of cmake.
    2. `cxx_standard` (int): The C++ standard whose features are requested to build this target.
    3. `enable_testing` (string): Enables the tests for the plugin. The only accepted variants are "ON" and "OFF".
4. **test**: Contains the details of the plugin tests.
    1. `mkdir` (bool): If true the test directory is created.
    2. `cmakelists` (bool): If true a CMakeLists file will be created for the tests.
    3. `cmake_min_required` (float): Require a minimum version of cmake.
    4. `tst_pluginloader` (bool): If true the pluginloader test is created.
5. **doc**: Contains the details of the plugin documentation.
    1. `mkdir` (bool): If true the doc directory is created.
6. **resources**: Contains the details of the plugin resources.
    1. `mkdir` (bool): If true the res directory is created.
    2. `resources_qrc` (bool): If true the resources_qrc file is created.

*All fields must be filled in.

## Scopy PDK

### Add new modules to the PDK package

If we want a new module from Scopy to be part of the package, then when installing libraries or directories in CMakeLists, the option `COMPONENT ${SCOPY_PDK}` must be added.

- libraries: `install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SCOPY_DLL_INSTALL_PATH} COMPONENT ${SCOPY_PDK} RUNTIME DESTINATION ${SCOPY_DLL_INSTALL_PATH})`.
- headers: `install(DIRECTORY include/ DESTINATION include/ COMPONENT ${SCOPY_PDK})`.
- resources: `install(DIRECTORY res/ DESTINATION resources/ COMPONENT ${SCOPY_PDK})`.

### Develop a new plugin using ScopyPluginRunner

1. Download the PDK package from the Scopy repository.
    - If the package is not available or if changes have been made to the Scopy libraries, then Scopy must be built locally with the `--target package` option.
        - In this case a `package` folder will be generated inside the Scopy build folder.
2. Generate the ScopyPluginRunner project using the `plugin_generator.py`.
    - Fill in the `config.json` file.
    - This project contains a `plugin` submodule where the desired plugin can be developed.
3. Choose an IDE and start working.
    - Our recommendation is to use QtCreator.

## Adding new config specifications

For fields like `plugin`, `tool` and `cmakelists` new specifications can be added.
A few steps must be completed:

1. Add the new specification in the desired field (add new fields in `config.json`).
2. Use the specification in the templates. Each template contains a `config` variable through which
the information from each field can be accessed. A specific syntax must be used because the script uses the
the mako library to render the templates. [Watch mako documentation page for more details.](https://docs.makotemplates.org/en/latest/)
    - the changes made in the `plugin` field will be reflected in the **plugin_src_template** and **plugin_header_template** template files
    - the changes made in the `tool` field will be reflected in the **tool_src_template** and **tool_header_template** template files
    - the changes made in the `cmakelists` field will be reflected in the **cmakelists_template**  template file
3. Run the `plugin_generator.py` script.
