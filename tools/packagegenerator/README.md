# Package Generator

## Overview

The `package_generator.py` script is a utility for generating and managing packages, plugins, and related resources for the Scopy project. It provides functionality to create package structures, generate plugins, handle translations, manage styles, and more.

## Platform

The `package_generator.py` script is designed to work on Linux, Windows, and macOS. Below are platform-specific details and instructions:

### Linux

- **Prerequisites**:
  - Ensure Python 3 is installed.
  - Install Git and other required tools using your package manager:

    ```bash
    sudo apt update && sudo apt install git python3-pip
    ```

  - Install the required Python dependencies:

    ```bash
    pip install -r requirements.txt
    ```

- **Running the Script**:
  - Use the terminal to navigate to the `scopy/tools/packagegenerator` directory and run:

    ```bash
    ./package_generator.py
    ```

### Windows

- **Prerequisites**:

  - Install Python 3 from [python.org](https://www.python.org/).
  - Ensure Git is installed and added to the system `PATH`.
  - Install the required Python dependencies:

    ```cmd
    pip install -r requirements.txt
    ```

- **Running the Script**:
  - Open a Command Prompt or PowerShell, navigate to the `scopy/tools/packagegenerator` directory, and run:

    ```cmd
    python package_generator.py
    ```

- **Note**:
  - On Windows, the script does not use POSIX file permissions (`MODE = 0o775`), so permissions-related functionality is skipped.

### macOS

- **Prerequisites**:
  - Ensure Python 3 is installed (can be installed via [Homebrew](https://brew.sh/)):

    ```bash
    brew install python3
    ```

  - Install Git:

    ```bash
    brew install git
    ```

  - Install the required Python dependencies:

    ```bash
    pip3 install -r requirements.txt
    ```

- **Running the Script**:
  - Use the terminal to navigate to the `scopy/tools/packagegenerator` directory and run:

    ```bash
    ./package_generator.py
    ```

- **Note**:
  - macOS handles file permissions similarly to Linux, so no additional adjustments are needed.

## Running the Script

### Basic Usage

1. If the script is run from the `scopy/tools/packagegenerator` directory:

    ```bash
    ./package_generator.py
    ```

2. Otherwise, specify the paths to the Scopy repository and configuration files:

    ```bash
    ./package_generator.py --scopy_path=path/to/scopy --config_file_path=path/to/pkg.json
    ```

For help, run:

```bash
./package_generator.py -h
```

### Example Commands

1. **Generate a complete package structure**:

    ```bash
    ./package_generator.py --all
    ```

2. **Generate a plugin using a specific configuration file**:

    ```bash
    ./package_generator.py -p /path/to/plugin.json
    ```

3. **Add translations to a package**:

    ```bash
    ./package_generator.py --translation
    ```

4. **Add style templates to a package**:

    ```bash
    ./package_generator.py --style
    ```

5. **Generate a Plugin Development Kit (PDK)**:

    ```bash
    ./package_generator.py --pdk
    ```

6. **Initialize a new Git submodule and generate a package**:

    ```bash
    ./package_generator.py --init
    ```

7. **Add an existing Git repository as a submodule**:

    ```bash
    ./package_generator.py --add_submodule <repository_url>
    ```

8. **Archive all packages in a directory**:

    ```bash
    ./package_generator.py -a --src /path/to/packages --dest /path/to/archives
    ```

---

## Configuration Files

### `pkg.json`

The `pkg.json` file contains the configuration details for generating packages. Below is an example:

```json
{
    "id": "newpackage",
    "title": "New package",
    "description": "A new package",
    "author": "Analog Devices Inc.",
    "license": "LGPL",
    "category": ["iio", "plugin"]
}
```

### `plugin.json`

The `plugin.json` file contains the configuration details for generating plugins. Below is an example:

```json
{
    "baseplugin": false,
    "plugin_name": "newplugin",
    "plugin_display_name": "New Plugin",
    "plugin_description": "A new plugin",
    "class_name": "NEWPlugin",
    "namespace": "scopy::newplugin",
    "device_category": "iio",
    "tools": [
        {
            "id": "newtool1",
            "tool_name": "Tool1",
            "file_name": "tool1",
            "class_name": "Tool1",
            "namespace": "scopy::newplugin"
        }
    ],
    "cmakelists": {
        "cmake_min_required": 3.9,
        "cxx_standard": 20,
        "enable_testing": "ON"
    },
    "style": {
        "qss": true,
        "json": false
    },
    "test": {
        "cmakelists": true,
        "cmake_min_required": 3.5,
        "tst_pluginloader": true
    },
    "resources": {
        "resources_qrc": true
    },
    "doc": true,
    "pdk": {
        "deps_path": "",
        "project_path": ""
    }
}
```

---

## Manifest File

The `manifest.json.cmakein` file is generated as part of the package structure. It contains metadata about the package and is used during the build process. Below is an example of its structure:

```json
{
    "id": "newpackage",
    "title": "New Package",
    "description": "A new package for Scopy",
    "author": "Analog Devices Inc.",
    "license": "LGPL",
    "category": ["iio", "plugin"]
}
```

The manifest file is automatically created during the package generation process and ensures that all necessary metadata is included for proper integration with Scopy.

---

## Packages development flow

### New package

1. Clone Scopy using:  

    ```bash
    git clone --branch <branch_or_tag> https://github.com/analogdevicesinc/scopy.git
    ```

2. Fill the `pkg.json` and `plugin.json` configuration files (see the [pkg.json example](#pkgjson) and [plugin.json example](#pluginjson) for guidance).

3. Create a package repository using:  

    ```bash
    ./package_generator.py --init
    ```

    - If you want to create a repository and a complete package structure call:

        ```bash
        ./package_generator.py --init --all
        ```

    - If you don't want to save the package sources inside a Git repository, you can directly generate the package structure using:

        ```bash
        ./package_generator.py --all
        ```

    - If the package contains only plugins, you can call:

        ```bash
        ./package_generator.py --plugin
        ```

4. Implement the package functionalities, build the package inside Scopy, and test it. After all the work is done, push the sources into the repository.

5. If you want to export the package, use the following command:

    ```bash
    ./package_generator.py -a --src /path/to/package/build/folder --dest /path/to/destination
    ```

    - Here, the "build folder" refers to the directory where the package was built, and the "destination" refers to the directory where the generated archive (zip file) will be saved.
    - The archived package can be installed in Scopy using the package manager.

### Existent package

1. Clone Scopy using:  

    ```bash
    git clone --branch <branch_or_tag> https://github.com/analogdevicesinc/scopy.git
    ```

2. Add the package as submodule using:  

    ```bash
    ./package_generator.py --add_submodule <repository_url>
    ```

3. Implement the package functionalities, build the package inside Scopy, and test it. After all the work is done, push the sources into the repository.

    - To generate a plugin structure for an existent package you must 

4. If you want to export the package, use the following command:  

    ```bash
    ./package_generator.py -a --src /path/to/package/build/folder --dest /path/to/destination
    ```

    - Here, the "build folder" refers to the directory where the package was built, and the "destination" refers to the directory where the generated archive (zip file) will be saved.
    - The archived package can be installed in Scopy using the package manager.

## Scopy PDK

### Adding New Modules to the PDK Package

To include a new module in the PDK package, ensure that the `COMPONENT ${SCOPY_PDK}` option is added when installing libraries or directories in `CMakeLists.txt`.

- **Libraries**:

    ```cmake
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SCOPY_DLL_INSTALL_PATH} COMPONENT ${SCOPY_PDK} RUNTIME DESTINATION ${SCOPY_DLL_INSTALL_PATH})
    ```

- **Headers**:

    ```cmake
    install(DIRECTORY include/ DESTINATION include/ COMPONENT ${SCOPY_PDK})
    ```

- **Resources**:

    ```cmake
    install(DIRECTORY res/ DESTINATION resources/ COMPONENT ${SCOPY_PDK})
    ```

### Developing a New Plugin Using ScopyPluginRunner

1. Download the PDK package from the Scopy repository.
    - If the package is unavailable or changes have been made to the Scopy libraries, build Scopy locally with the `--target package` option.
    - This will generate a `package` folder inside the Scopy build directory.

2. Generate the ScopyPluginRunner project using `package_generator.py`.
    - Fill in the `plugin.json` file.
    - The project will contain a `plugin` submodule where the desired plugin can be developed.

3. Choose an IDE and start working.
    - **Recommendation**: Use QtCreator.

---

## Adding New Config Specifications

To add new specifications for fields like `plugin`, `tool`, or `cmakelists`, follow these steps:

1. Add the new specification to the desired field in `pkg.json` or `plugin.json`.
2. Use the specification in the templates. Each template contains a `config` variable to access the field's information. Use the [Mako template syntax](https://docs.makotemplates.org/en/latest/) for rendering.
    - Changes to the `plugin` field will affect the `plugin_src` and `plugin_header` files.
    - Changes to the `tool` field will affect the `tool_src` and `tool_header` files.
    - Changes to the `cmakelists` field will affect the `cmakelists` file.
3. Run the `package_generator.py` script to apply the changes.