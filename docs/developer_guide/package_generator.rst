.. _package_generator:


Package Generator
================================================================================


Chapters

- `1. Overview <#overview>`_
- `2. Platform <#platform>`_
- `3. Running the Script <#running-the-script>`_
- `4. Configuration Files <#configuration-files>`_
- `5. Manifest File <#manifest-file>`_
- `6. Packages development flow <#packages-development-flow>`_
- `7. Scopy PDK <#scopy-pdk>`_
- `8. Adding New Config Specifications <#adding-new-config-specifications>`_


Overview
--------------------------------------------------------------------------------
The `package_generator.py` script is a utility for generating and managing packages, 
plugins, and related resources for the Scopy project. It provides functionality to 
create package structures, generate plugins, handle translations, manage styles, and more.

Platform
--------------------------------------------------------------------------------
The `package_generator.py` script is designed to work on Linux, Windows, and macOS. 
Below are platform-specific details and instructions:

**Linux**

- **Prerequisites**:
  - Ensure Python 3 is installed.
  - Install Git and other required tools using your package manager:

    .. code-block:: bash

        sudo apt update && sudo apt install git python3-pip

  - Install the required Python dependencies:

    .. code-block:: bash

        pip install -r requirements.txt

- **Running the Script**:
  - Use the terminal to navigate to the `scopy/tools/packagegenerator` directory and run:

    .. code-block:: bash

        ./package_generator.py

**Windows**

- **Prerequisites**:
  - Install Python 3 from https://www.python.org/.
  - Ensure Git is installed and added to the system `PATH`.
  - Install the required Python dependencies:

    .. code-block:: bash

        pip install -r requirements.txt

- **Running the Script**:
  - Open a Command Prompt or PowerShell, navigate to the `scopy/tools/packagegenerator` directory, and run:

    .. code-block:: bash

        python package_generator.py

- **Note**:
  - On Windows, the script does not use POSIX file permissions (`MODE = 0o775`), so permissions-related functionality is skipped.

**macOS**

- **Prerequisites**:
  - Ensure Python 3 is installed (can be installed via Homebrew: https://brew.sh/):

    .. code-block:: bash

        brew install python3

  - Install Git:

    .. code-block:: bash

        brew install git

  - Install the required Python dependencies:

    .. code-block:: bash

        pip3 install -r requirements.txt

- **Running the Script**:
  - Use the terminal to navigate to the `scopy/tools/packagegenerator` directory and run:

    .. code-block:: bash

        ./package_generator.py

- **Note**:
  - macOS handles file permissions similarly to Linux, so no additional adjustments are needed.

Running the Script
--------------------------------------------------------------------------------
**Basic Usage**

1. If the script is run from the `scopy/tools/packagegenerator` directory:

    .. code-block:: bash

        ./package_generator.py

2. Otherwise, specify the paths to the Scopy repository and configuration files:

    .. code-block:: bash

        ./package_generator.py --scopy_path=path/to/scopy --config_file_path=path/to/pkg.json

For help, run:

.. code-block:: bash

    ./package_generator.py -h

**Example Commands**

1. Generate a complete package structure:

    .. code-block:: bash

        ./package_generator.py --all

2. Generate a plugin using a specific configuration file:

    .. code-block:: bash

        ./package_generator.py -p /path/to/plugin.json

3. Add translations to a package:

    .. code-block:: bash

        ./package_generator.py --translation

4. Add style templates to a package:

    .. code-block:: bash

        ./package_generator.py --style

5. Generate a Plugin Development Kit (PDK):

    .. code-block:: bash

        ./package_generator.py --pdk

6. Initialize a new Git submodule and generate a package:

    .. code-block:: bash

        ./package_generator.py --init

7. Add an existing Git repository as a submodule:

    .. code-block:: bash

        ./package_generator.py --add_submodule <repository_url>

8. Archive all packages in a directory:

    .. code-block:: bash

        ./package_generator.py -a --src /path/to/packages --dest /path/to/archives

Configuration Files
--------------------------------------------------------------------------------
**pkg.json**

The `pkg.json` file contains the configuration details for generating packages. Example:

.. code-block:: json

    {
        "id": "newpackage",
        "title": "New package",
        "description": "A new package",
        "author": "Analog Devices Inc.",
        "license": "LGPL",
        "category": ["iio", "plugin"]
    }

**plugin.json**

The `plugin.json` file contains the configuration details for generating plugins. Example:

.. code-block:: json

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

Manifest File
--------------------------------------------------------------------------------
The `manifest.json.cmakein` file is generated as part of the package structure. It contains metadata about the package and is used during the build process. Example:

.. code-block:: json

    {
        "id": "newpackage",
        "title": "New Package",
        "description": "A new package for Scopy",
        "author": "Analog Devices Inc.",
        "license": "LGPL",
        "category": ["iio", "plugin"]
    }

The manifest file is automatically created during the package generation process and ensures that all necessary metadata is included for proper integration with Scopy.

Packages development flow
--------------------------------------------------------------------------------
**New package**

1. Clone Scopy using:

    .. code-block:: bash

        git clone --branch <branch_or_tag> https://github.com/analogdevicesinc/scopy.git

2. Fill the `pkg.json` and `plugin.json` configuration files (see the pkg.json and plugin.json examples above).

3. Create a package repository using:

    .. code-block:: bash

        ./package_generator.py --init

    - To create a repository and a complete package structure:

        .. code-block:: bash

            ./package_generator.py --init --all

    - To generate the package structure without a Git repository:

        .. code-block:: bash

            ./package_generator.py --all

    - If the package contains only plugins:

        .. code-block:: bash

            ./package_generator.py --plugin

4. Implement the package functionalities, build the package inside Scopy, and test it. After all the work is done, push the sources into the repository.

5. To export the package:

    .. code-block:: bash

        ./package_generator.py -a --src /path/to/package/build/folder --dest /path/to/destination

    - The "build folder" is where the package was built, and the "destination" is where the generated archive (zip file) will be saved.
    - The archived package can be installed in Scopy using the package manager.

**Existent package**

1. Clone Scopy using:

    .. code-block:: bash

        git clone --branch <branch_or_tag> https://github.com/analogdevicesinc/scopy.git

2. Add the package as submodule:

    .. code-block:: bash

        ./package_generator.py --add_submodule <repository_url>

3. Implement the package functionalities, build the package inside Scopy, and test it. After all the work is done, push the sources into the repository.

4. To export the package:

    .. code-block:: bash

        ./package_generator.py -a --src /path/to/package/build/folder --dest /path/to/destination

    - The "build folder" is where the package was built, and the "destination" is where the generated archive (zip file) will be saved.
    - The archived package can be installed in Scopy using the package manager.

Scopy PDK
--------------------------------------------------------------------------------
**Adding New Modules to the PDK Package**

To include a new module in the PDK package, ensure that the `COMPONENT ${SCOPY_PDK}` option is added when installing libraries or directories in `CMakeLists.txt`.

- **Libraries**:

    .. code-block:: cmake

        install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SCOPY_DLL_INSTALL_PATH} COMPONENT ${SCOPY_PDK} RUNTIME DESTINATION ${SCOPY_DLL_INSTALL_PATH})

- **Headers**:

    .. code-block:: cmake

        install(DIRECTORY include/ DESTINATION include/ COMPONENT ${SCOPY_PDK})

- **Resources**:

    .. code-block:: cmake

        install(DIRECTORY res/ DESTINATION resources/ COMPONENT ${SCOPY_PDK})

**Developing a New Plugin Using ScopyPluginRunner**

1. Download the PDK package from the Scopy repository.
    - If the package is unavailable or changes have been made to the Scopy libraries, build Scopy locally with the `--target package` option.
    - This will generate a `package` folder inside the Scopy build directory.

2. Generate the ScopyPluginRunner project using `package_generator.py`.
    - Fill in the `plugin.json` file.
    - The project will contain a `plugin` submodule where the desired plugin can be developed.

3. Choose an IDE and start working.
    - **Recommendation**: Use QtCreator.

Adding New Config Specifications
--------------------------------------------------------------------------------
To add new specifications for fields like `plugin`, `tool`, or `cmakelists`, follow these steps:

1. Add the new specification to the desired field in `pkg.json` or `plugin.json`.
2. Use the specification in the templates. Each template contains a `config` variable to access the field's information. Use the Mako template syntax (https://docs.makotemplates.org/en/latest/) for rendering.
    - Changes to the `plugin` field will affect the `plugin_src` and `plugin_header` files.
    - Changes to the `tool` field will affect the `tool_src` and `tool_header` files.
    - Changes to the `cmakelists` field will affect the `cmakelists` file.
3. Run the `package_generator.py` script to apply the changes.


