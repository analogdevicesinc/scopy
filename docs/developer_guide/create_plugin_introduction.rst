.. _create_plugin_introduction:

Introduction
--------------------------------------------------------------------------------
This tutorial will guide you through the process of creating a plugin for Scopy. 
Each chapter will cover a specific aspect of plugin development, from creating a new plugin 
to adding advanced features like compatibility conditions and formatting scripts.

Chapters
--------------------------------------------------------------------------------

- `How to create a package <#how-to-create-a-package>`_
- `Using Connection Provider <#using-connection-provider>`_
- `How Do We Determine When the Plugin Will Show (Compatibility Condition) <#how-do-we-determine-when-the-plugin-will-show-compatibility-condition>`_
- `How to Filter Between Compatible Plugins <#how-to-filter-between-compatible-plugins>`_
- `Setting Tool Icon and Title <#setting-tool-icon-and-title>`_
- `How to Add Resources <#how-to-add-resources>`_
- `How to Create / Edit Preferences for the Plugin <#how-to-create--edit-preferences-for-the-plugin>`_
- `How to Add Compatible Plugins <#how-to-add-compatible-plugins>`_
- `Useful Tools <#useful-tools>`_
- `IIOWidgets <#iiowidgets>`_
- `Menu UI Items <#menu-ui-items>`_
- `How to Create a Device Icon <#how-to-create-a-device-icon>`_
- `How to Create a Tutorial for Your Plugin <#how-to-create-a-tutorial-for-your-plugin>`_

How to create a package
================================================================================

1. **Package Generator**
================================================================================

Overview
--------------------------------------------------------------------------------
The `package_generator.py` script is a utility for generating and managing packages, plugins, and related resources for the Scopy project. It provides functionality to create package structures, generate plugins, handle translations, manage styles, and more.

Platform
--------------------------------------------------------------------------------
The `package_generator.py` script is designed to work on Linux, Windows, and macOS. Below are platform-specific details and instructions:

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

How to create a plugin
================================================================================

2. **Using Connection Provider**
--------------------------------------------------------------------------------

Understand how to use the connection provider to interact with external
systems or data sources.

To access the device, we use the `ConnectionProvider` class. This class is
available by adding:

.. code-block:: cpp

    #include <iioutil/connectionprovider.h>

To get a connection to the hardware in your plugin, you can use the following
code:

.. code-block:: cpp

    Connection *conn = ConnectionProvider::open(m_param);

Here, `m_param` is provided by the plugin.

To be able to access the context from the connection, use the following code:

.. code-block:: cpp

    conn->context();

3. **How Do We Determine When the Plugin Will Show (Compatibility Condition)**
--------------------------------------------------------------------------------

To determine if the plugin you created will show as an option for a device,
you need to create a compatibility condition. This is done inside the
`compatible` function, depending on the restrictions of your plugin. Here,
you should define a condition for when the plugin should show.

For example, if we are trying to determine if a device has `"ad9361-phy"` as
a device, we would use:

.. code-block:: cpp

    bool ret = false;
    Connection *conn = ConnectionProvider::open(m_param);

    if (!conn) {
        qWarning(CAT_PLUTOPLUGIN) << "No context available ";
        return false;
    }
    iio_device *plutoDevice = iio_context_find_device(conn->context(), "ad9361-phy");
    if (plutoDevice) {
        ret = true;
    }

    ConnectionProvider::close(m_param);

    return ret;

This function checks if the device context contains `"ad9361-phy"`. If it
does, the plugin will be marked as compatible and will show as an option for
the device.

4. **How to Filter Between Compatible Plugins**
--------------------------------------------------------------------------------

Inside the `initMetadata()` function, you can define specific metadata to
filter and prioritize plugins. This metadata includes the following fields:

- **Priority**: Determines the loading order of plugins. Plugins with higher
  priority values are loaded earlier. This is used to ensure that critical
  plugins are initialized first or in a specific order relative to other
  plugins.
- **Category**: Specifies the categories to which the plugin belongs. This is
  used to classify the plugin and determine for which contexts or purposes
  the plugin should be considered.
- **Exclude**: Determines which plugins should not show if this one is
  displayed. For example:

  .. code-block:: json

      "exclude": ["*"]

  This means only this plugin will show. Alternatively:

  .. code-block:: json

      "exclude": ["datalogger"]

  This means all compatible plugins will show except the `datalogger` plugin.

  You can also use `!` in front of a plugin name in the `exclude` list to
  ensure it will show. For example:

  .. code-block:: json

      "exclude": ["*", "!datalogger"]

  This means only your plugin and the `datalogger` plugin will show.

By setting these fields appropriately, you can control the visibility and
priority of your plugin relative to others.

5. **Setting Tool Icon and Title**
--------------------------------------------------------------------------------

To have your plugin's tools appear in the left menu among others, you need to
set a title and an icon for it. Since a plugin can have multiple tools, each
tool needs to be considered during this process.

Inside the `loadToolList()` function, for each tool, you need to add the
following:

.. code-block:: cpp

    m_toolList.append(
        SCOPY_NEW_TOOLMENUENTRY(TOOL_ID, TOOL_DISPLAY_NAME, TOOL_ICON));

Note that this will not create the tool itself, only the menu entry for your
tool.

To associate the menu entry with a tool, you need to create the tool and set
it for the corresponding menu entry inside the `onConnect()` function. For
example:

.. code-block:: cpp

    MyTool *myTool = new MyTool();
    m_toolList[0]->setTool(myTool);
    m_toolList[0]->setEnabled(true);
    m_toolList[0]->setRunBtnVisible(true);

This will:
- Create the tool (`MyTool` in this example).
- Associate it with the menu entry you defined at index `0`.
- Enable the tool's run button.
- Ensure the tool menu action is enabled.

By following these steps, you can ensure that your plugin's tools are
properly displayed and functional in the Scopy interface.

6. **How to Add Resources**
--------------------------------------------------------------------------------

To add resource files to your plugin, you first need to place the file in the
`resources` folder of your project. Then, in the same folder, locate the
`resources.qrc` file and add the resource name to it.

For example, let's add `test.svg` to our project resources. The
`resources.qrc` file would look like this:

.. code-block:: xml

    <RCC>
        <qresource prefix="/myPlugin">
            <file>test.svg</file>
        </qresource>
    </RCC>

Now that we have added the file to our resources, we can use it in the plugin
by referencing it as follows:

.. code-block:: cpp

    ";/myPlugin/test.svg"

By following these steps, you can include and use additional resource files,
such as images, or other assets, in your plugin.

7. **How to Create / Edit Preferences for the Plugin**
--------------------------------------------------------------------------------

Plugin preferences are settings that retain their values between runs. These
preferences are available before connecting to a device and are displayed in
the bottom left corner of the application.

To add preferences to your plugin, you need to include the following headers:

.. code-block:: cpp

    #include <pluginbase/preferences.h>
    #include <gui/preferenceshelper.h>

Additionally, you need to implement the functions `initPreferences()` and
`loadPreferencesPage()`.

### Adding a Preference
Let's add a preference to our plugin to store the read interval value.

**1. Initialize Preferences**

Inside the `initPreferences()` function, you set the default values for your
preferences. Each preference has a unique ID that starts with the plugin
name. In our case, it will be `myplugin_read_interval`.

.. code-block:: cpp

    void MyPlugin::initPreferences()
    {
        Preferences *p = Preferences::GetInstance();
        p->init("myplugin_read_interval", "1");
    }

**2. Display Preferences in Scopy**

To display the preference under Scopy Preferences, you need to implement the
`loadPreferencesPage()` function. Inside this function, you create a
`QWidget` that contains the preferences. You can use predefined macros for
creating different types of preferences.

.. code-block:: cpp

    bool MyPlugin::loadPreferencesPage()
    {
        Preferences *p = Preferences::GetInstance();

        QWidget *preferencesPage = new QWidget();
        QVBoxLayout *lay = new QVBoxLayout(preferencesPage);
        lay->addWidget(PREFERENCE_EDIT_VALIDATION(
            p, "myplugin_read_interval", "Read interval (seconds)",
            "Select the time interval, in seconds, for data polling in the instrument.",
            [](const QString &text) {
                // Check if input is a positive integer
                bool ok;
                auto value = text.toInt(&ok);
                return ok && value >= 0;
            },
            preferencesPage));
        return true;
    }

### Available Preference Macros
The following macros are available for creating preferences:

.. code-block:: cpp

    #define PREFERENCE_CHECK_BOX(p, id, title, description, parent) \
        PreferencesHelper::addPreferenceCheckBox(p, id, title, description, parent)

    #define PREFERENCE_EDIT(p, id, title, description, parent) \
        PreferencesHelper::addPreferenceEdit(p, id, title, description, parent)

    #define PREFERENCE_EDIT_VALIDATION(p, id, title, description, validator, parent) \
        PreferencesHelper::addPreferenceEditValidation(p, id, title, description, validator, parent)

    #define PREFERENCE_COMBO(p, id, title, description, options, parent) \
        PreferencesHelper::addPreferenceCombo(p, id, title, description, options, parent)

    #define PREFERENCE_COMBO_LIST(p, id, title, description, options, parent) \
        PreferencesHelper::addPreferenceComboList(p, id, title, description, options, parent)

### Accessing Preferences
You can access and use preferences in your code. For example, to access a
preference called `myplugin_test`, you can do the following:

.. code-block:: cpp

    Preferences *p = Preferences::GetInstance();
    auto testPreference = p->get("myplugin_test");

### Reacting to Preference Changes
If you want to update your code when the value of a preference changes, you
can use the `preferenceChanged` signal from the `Preferences` class.

8. **How to Add Compatible Plugins**
--------------------------------------------------------------------------------

If you want to use parts of other plugins in your plugin, you first need to
modify your plugin's `CMakeLists.txt` file.

### Adding a Library to Your Plugin

Under the `target_link_libraries` section of your `CMakeLists.txt`, add the
library you want to use. For example, if you want to add access to elements
from `scopy-test` to your plugin, the code will look like this:

.. code-block:: cmake

    target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC Qt::Widgets
               Qt::Core
               scopy-pluginbase
               scopy-gui
               scopy-iioutil
               scopy-iio-widgets
               scopy-test
    )

By adding the desired library to the `target_link_libraries` section, you
can access its functionality in your plugin.

Ensure that the library you are linking is available and properly configured
in your Scopy environment.

9. **Useful Tools**
--------------------------------------------------------------------------------

We have a couple of tools designed to make some of your work easier. Among
these tools are a script for code formatting and a script for license
headers. It is recommended that, before making a pull request, you run these
two scripts.

### License Header Script

This script should be run when creating new files to ensure they include the
appropriate license headers. To run it, use the following command:

.. code-block:: bash

    ./tools/license.sh

### Formatting Script

The formatting script ensures that your code follows our coding guidelines.
To run it, use the following command:

.. code-block:: bash

    ./tools/format.sh

10. **IIOWidgets**
--------------------------------------------------------------------------------

`IIOWidgets` are modular UI components in Scopy designed to facilitate
interaction with IIO (Industrial I/O) devices. They provide a graphical
interface for displaying and managing data from these devices, making it
easier for users to interact with hardware.

**Key Features of IIOWidgets**

- **Data Interaction**:
  `IIOWidgets` use data strategies (`DataStrategyInterface`) to handle
  reading and writing data from IIO devices. They support asynchronous
  operations such as `readAsync()` and `writeAsync()` to ensure smooth data
  handling without blocking the UI.
- **UI Customization**:
  `IIOWidgets` use UI strategies (`GuiStrategyInterface`) to define their
  appearance and behavior. This allows developers to create widgets that are
  tailored to specific use cases or device requirements.
- **Integration with Scopy**:
  `IIOWidgets` are seamlessly integrated into the Scopy application,
  allowing them to be used alongside other tools and plugins. They can be
  added to the interface using the `IIOWidgetBuilder`.
- **Progress and Status Indicators**:
  Many `IIOWidgets` include built-in progress bars and status indicators to
  provide real-time feedback to users during operations.
- **Resource Management**:
  Developers can customize `IIOWidgets` with titles, attributes, and other
  parameters using the `IIOWidgetBuilder`. This ensures that widgets are
  properly configured for their intended purpose.

**Creating an IIOWidget**

Below are two examples of creating `IIOWidgets` using the `IIOWidgetBuilder`:

**1. Creating a Single IIOWidget for a Specific Device Attribute**

To create a single `IIOWidget` for a specific device attribute, you can use
the following code:

.. code-block:: cpp

    IIOWidget *attributeWidget = IIOWidgetBuilder(parent)
                                 .device(myDevice)
                                 .attribute("specific_attribute_name")
                                 .uiStrategy(IIOWidgetBuilder::RangeUi)
                                 .infoMessage("This is an informational message")
                                 .title("Attribute Title")
                                 .buildSingle();

In this example:
- `device()` specifies the device to which the widget is linked.
- `attribute()` specifies the attribute of the device to be controlled or displayed.
- `uiStrategy()` defines the UI strategy for the widget (e.g., `RangeUi`).
- `infoMessage()` adds an informational message for the widget.
- `title()` sets the title of the widget.
- `buildSingle()` creates the widget.

**2. Creating All IIOWidgets for a Channel of a Device**

To create all `IIOWidgets` for a specific channel of a device, you can use
the following code:

.. code-block:: cpp

    QList<IIOWidget *> channelWidgets = 
        IIOWidgetBuilder(parent)
            .channel(myChannel)
            .buildAll();

In this example:
- `channel()` specifies the channel of the device for which widgets are created.
- `buildAll()` automatically creates widgets for all attributes of the specified channel.

**UI Strategy Assignment**

- When using `buildAll()`, the UI strategy for each widget is assigned
  automatically based on the attribute type and metadata.
- When using `buildSingle()`, you can manually specify the UI strategy using
  the `uiStrategy()` method.

By leveraging these methods, developers can create both individual and
grouped `IIOWidgets` to suit their specific use cases.

**Benefits of Using IIOWidgets**

- Simplifies the development of user interfaces for IIO devices.
- Provides a consistent look and feel across different plugins.
- Reduces the amount of boilerplate code needed to create UI components.
- Supports advanced features like asynchronous data handling and progress
  indicators.

By leveraging `IIOWidgets`, developers can create powerful and user-friendly
interfaces for interacting with IIO devices in Scopy.

11. **Menu UI Items**
--------------------------------------------------------------------------------

Scopy provides a variety of `Menu` UI components to help developers build
menus, settings panels, and other hierarchical UI structures. For details on
available menu components and their usage, please refer to the relevant
documentation or code examples.

12. **How to Create a Device Icon**
--------------------------------------------------------------------------------

To create a custom device icon for your plugin, you can use the
`DeviceIconBuilder` class. This allows you to combine images and labels to
create a visually distinctive icon for your device in the Scopy interface.

Below is a generic example of how to implement a device icon in your plugin:

.. code-block:: cpp

    bool PlutoPlugin::loadIcon()
    {
        QLabel *logo = new QLabel();
        QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
        int pixmapHeight = 14;
        pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
        logo->setPixmap(pixmap);

        QLabel *footer = new QLabel("DEVICE_NAME");
        Style::setStyle(footer, style::properties::label::deviceIcon, true);

        m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE)
            .headerWidget(logo)
            .footerWidget(footer)
            .build();

        return true;
    }

In this example:
- A `QLabel` is used to display the device logo, loaded from a resource file and scaled to the desired height.
- Another `QLabel` is used as a footer to display a device name or model.
- The `DeviceIconBuilder` is used to assemble the icon, specifying the shape, header, and footer widgets.
- The resulting icon is assigned to `m_icon`.

You can customize the icon by changing the image, label text, or shape as
needed for your device.

13. **How to Create a Tutorial for Your Plugin**
--------------------------------------------------------------------------------

Scopy supports interactive tutorials for plugins, allowing you to guide users
through your tool's features step by step. Tutorials are defined using JSON
files and integrated into the GUI using the tutorial overlay system.

**Step 1: Create a Tutorial Chapters JSON File**

In your plugin’s ``res`` or ``resources`` folder, create a file named
``tutorial_chapters.json``. Each tutorial consists of one or more chapters,
each describing a UI element and its purpose.

Example structure:

.. code-block:: json

    {
      "mytool": [
        {
          "index": 1,
          "names": ["RUN_BUTTON"],
          "description": "Click to start the tool.",
          "anchor": "HP_BOTTOM",
          "content": "HP_BOTTOM",
          "y_offset": 10
        },
        {
          "index": 2,
          "names": ["SETTINGS_BUTTON"],
          "description": "Open the settings for more options.",
          "anchor": "HP_BOTTOM",
          "content": "HP_BOTTOM",
          "y_offset": 10
        }
      ]
    }

- ``names``: List of widget object names to highlight.
- ``description``: Text shown to the user.
- ``anchor``/``content``: Positioning for the overlay (e.g., ``HP_BOTTOM``,
  ``HP_TOP``).
- ``y_offset``, ``x_offset``: Optional, for fine-tuning overlay position.

**Step 2: Reference the Tutorial in Your Plugin**

Ensure your plugin loads the tutorial chapters file and connects it to the
tool's UI. The Scopy GUI uses classes like ``TutorialOverlay`` and
``TutorialChapter`` to display these steps.

**Step 3: Use the Tutorial Overlay in Code**

In your tool's code, you can trigger the tutorial overlay, for example:

.. code-block:: cpp

    #include <gui/tutorialoverlay.h>
    // ...existing code...
    TutorialOverlay *tutorial = new TutorialOverlay(parentWidget);
    tutorial->addChapter({runButton},
        "Click to start the tool.",
        parentWidget, 0, 10,
        HoverPosition::HP_BOTTOM, HoverPosition::HP_BOTTOM);
    // Add more chapters as needed
    tutorial->start();

Or, if your plugin is already set up for tutorials, simply ensure the JSON
file is present and correctly formatted.

**Step 4: Test Your Tutorial**

Build and run Scopy, open your plugin, and start the tutorial. The overlay
should highlight the specified widgets and display your descriptions.

**Tips:**

- Use meaningful widget names in your UI code so they match the ``names`` in
  the JSON.
- You can localize or update descriptions easily by editing the JSON file.
- See other plugins' ``tutorial_chapters.json`` for more examples.

Conclusion
--------------------------------------------------------------------------------
By following these chapters, you will gain a comprehensive understanding of how to create, 
customize, and deploy plugins in Scopy.




