.. _create_plugin_introduction:

How to create a plugin
================================================================================

Introduction
--------------------------------------------------------------------------------
This tutorial will guide you through the process of creating a plugin for Scopy. 
Each chapter will cover a specific aspect of plugin development, from creating a new plugin 
to adding advanced features like compatibility conditions and formatting scripts.

Chapters
--------------------------------------------------------------------------------

1. **Create a New Plugin**
--------------------------------------------------------------------------------

   This guide provides instructions on how to use the Scopy package generator, including the plugin generator.

   ### Pre-requisites

   Ensure you have the necessary dependencies installed:

   .. code-block:: bash

       pip install -r requirements.txt

   ### Config.json Attributes Explanation

   The `config.json` file contains various attributes that define the plugin configuration. Here is an explanation of each attribute:

   - **pdk**: Contains the configuration options of the plugin development tool.
       - `enable` (bool): If true, a ScopyPluginRunner project will be created at `project_path`.
       - `deps_path` (string): The path to the Scopy PDK package (the package must be unzipped).
       - `project_path` (string): The path to the directory where the PluginRunner project is created.
   - **plugin**: Contains details specific to the plugin.
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
   - **cmakelists**: Contains the details of the CMakeLists.
       - `cmake_min_required` (float): Require a minimum version of CMake.
       - `cxx_standard` (int): The C++ standard whose features are requested to build this target.
       - `enable_testing` (string): Enables the tests for the plugin. The only accepted variants are "ON" and "OFF".
   - **test**: Contains the details of the plugin tests.
       - `mkdir` (bool): If true, the test directory is created.
       - `cmakelists` (bool): If true, a CMakeLists file will be created for the tests.
       - `cmake_min_required` (float): Require a minimum version of CMake.
       - `tst_pluginloader` (bool): If true, the pluginloader test is created.
   - **doc**: Contains the details of the plugin documentation.
       - `mkdir` (bool): If true, the doc directory is created.
   - **resources**: Contains the details of the plugin resources.
       - `mkdir` (bool): If true, the res directory is created.
       - `resources_qrc` (bool): If true, the resources_qrc file is created.

   **All fields must be filled in.**

   ### Running the Plugin Generator Script

   The plugin generator script can be run in two ways:

   1. If it is run from "scopy/tools/plugingenerator":

      .. code-block:: bash

          ./plugin_generator.py

   2. Otherwise:

      .. code-block:: bash

          python plugin_generator.py --scopy_path=path/to/scopy --config_file_path=path/to/config/file.json

   For help, use:

   .. code-block:: bash

       python plugin_generator.py -h

   ### After Running the Script

   After running the script, you must insert the new plugin in `plugins/CMakeLists.txt`:

   .. code-block:: cmake

       option(ENABLE_PLUGIN_NEWPLUGIN "Enable NEWPLUGIN plugin" ON)
       if(ENABLE_PLUGIN_NEWPLUGIN)
       add_subdirectory(new)
       list(APPEND PLUGINS ${PLUGIN_NAME})
       endif()

   By following these steps, you can create the basic structure of a new plugin and implement the required methods.

2. **Using Connection Provider**
--------------------------------------------------------------------------------

   Understand how to use the connection provider to interact with external systems or data sources.

   To access the device, we use the `ConnectionProvider` class. This class is available by adding:

   .. code-block:: cpp

       #include <iioutil/connectionprovider.h>

   To get a connection to the hardware in your plugin, you can use the following code:

   .. code-block:: cpp

       Connection *conn = ConnectionProvider::open(m_param);

   Here, `m_param` is provided by the plugin.

   To be able to access the context from the connection, use the following code:

   .. code-block:: cpp

       conn->context();

3. **How Do We Determine When the Plugin Will Show (Compatibility Condition)**
--------------------------------------------------------------------------------

   To determine if the plugin you created will show as an option for a device, you need to create 
   a compatibility condition. This is done inside the `compatible` function, depending on the 
   restrictions of your plugin. Here, you should define a condition for when the plugin should show.

   For example, if we are trying to determine if a device has `"ad9361-phy"` as a device, we would use:

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

   This function checks if the device context contains `"ad9361-phy"`. If it does, the plugin will 
   be marked as compatible and will show as an option for the device.

4. **How to Filter Between Compatible Plugins**
--------------------------------------------------------------------------------

   Inside the `initMetadata()` function, you can define specific metadata to filter and prioritize plugins. 
   This metadata includes the following fields:

   - **Priority**: Determines the loading order of plugins. Plugins with higher priority values are loaded earlier. 
     This is used to ensure that critical plugins are initialized first or in a specific order relative to other plugins.

   - **Category**: Specifies the categories to which the plugin belongs. This is used to classify the plugin and 
     determine for which contexts or purposes the plugin should be considered.

   - **Exclude**: Determines which plugins should not show if this one is displayed. For example:
     
     .. code-block:: json

         "exclude": ["*"]

     This means only this plugin will show. Alternatively:

     .. code-block:: json

         "exclude": ["datalogger"]

     This means all compatible plugins will show except the `datalogger` plugin.

     You can also use `!` in front of a plugin name in the `exclude` list to ensure it will show. For example:

     .. code-block:: json

         "exclude": ["*", "!datalogger"]

     This means only your plugin and the `datalogger` plugin will show.

   By setting these fields appropriately, you can control the visibility and priority of your plugin relative to others.

5. **Setting Tool Icon and Title**
--------------------------------------------------------------------------------

   To have your plugin's tools appear in the left menu among others, you need to set a title and an icon for it. 
   Since a plugin can have multiple tools, each tool needs to be considered during this process.

   Inside the `loadToolList()` function, for each tool, you need to add the following:

   .. code-block:: cpp

       m_toolList.append(
           SCOPY_NEW_TOOLMENUENTRY(TOOL_ID, TOOL_DISPLAY_NAME, TOOL_ICON));

   Note that this will not create the tool itself, only the menu entry for your tool.

   To associate the menu entry with a tool, you need to create the tool and set it for the corresponding menu entry 
   inside the `onConnect()` function. For example:

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

   By following these steps, you can ensure that your plugin's tools are properly displayed and functional in the Scopy interface.

6. **How to Add Resources**
--------------------------------------------------------------------------------

   To add resource files to your plugin, you first need to place the file in the `resources` folder of your project. 
   Then, in the same folder, locate the `resources.qrc` file and add the resorce name to it.

   For example, let's add `test.svg` to our project resources. The `resources.qrc` file would look like this:

   .. code-block:: xml

       <RCC>
           <qresource prefix="/myPlugin">
               <file>test.svg</file>
           </qresource>
       </RCC>

   Now that we have added the file to our resources, we can use it in the plugin by referencing it as follows:

   .. code-block:: cpp

       ";/myPlugin/test.svg"

   By following these steps, you can include and use additional resource files, such as images, or other assets, in your plugin.

7. **How to Create / Edit Preferences for the Plugin**
--------------------------------------------------------------------------------

   Plugin preferences are settings that retain their values between runs. These preferences are available before connecting to a device and are displayed in the bottom left corner of the application.

   To add preferences to your plugin, you need to include the following headers:

   .. code-block:: cpp

       #include <pluginbase/preferences.h>
       #include <gui/preferenceshelper.h>

   Additionally, you need to implement the functions `initPreferences()` and `loadPreferencesPage()`.

   ### Adding a Preference
   Let's add a preference to our plugin to store the read interval value.

   **1. Initialize Preferences**

   Inside the `initPreferences()` function, you set the default values for your preferences. Each preference has a unique ID that starts with the plugin name. In our case, it will be `myplugin_read_interval`.

   .. code-block:: cpp

       void MyPlugin::initPreferences()
       {
           Preferences *p = Preferences::GetInstance();
           p->init("myplugin_read_interval", "1");
       }

   **2. Display Preferences in Scopy**

   To display the preference under Scopy Preferences, you need to implement the `loadPreferencesPage()` function. Inside this function, you create a `QWidget` that contains the preferences. You can use predefined macros for creating different types of preferences.

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

       #define PREFERENCE_CHECK_BOX(p, id, title, description, parent)                                                        \
           PreferencesHelper::addPreferenceCheckBox(p, id, title, description, parent)

       #define PREFERENCE_EDIT(p, id, title, description, parent)                                                             \
           PreferencesHelper::addPreferenceEdit(p, id, title, description, parent)

       #define PREFERENCE_EDIT_VALIDATION(p, id, title, description, validator, parent)                                       \
           PreferencesHelper::addPreferenceEditValidation(p, id, title, description, validator, parent)

       #define PREFERENCE_COMBO(p, id, title, description, options, parent)                                                   \
           PreferencesHelper::addPreferenceCombo(p, id, title, description, options, parent)

       #define PREFERENCE_COMBO_LIST(p, id, title, description, options, parent)                                              \
           PreferencesHelper::addPreferenceComboList(p, id, title, description, options, parent)

   ### Accessing Preferences
   You can access and use preferences in your code. For example, to access a preference called `myplugin_test`, you can do the following:

   .. code-block:: cpp

       Preferences *p = Preferences::GetInstance();
       auto testPreference = p->get("myplugin_test");

   ### Reacting to Preference Changes
   If you want to update your code when the value of a preference changes, you can use the `preferenceChanged` signal from the `Preferences` class.

8. **How to Add Compatible Plugins**
--------------------------------------------------------------------------------

   If you want to use parts of other plugins in your plugin, you first need to modify your plugin's `CMakeLists.txt` file.

   ### Adding a Library to Your Plugin

   Under the `target_link_libraries` section of your `CMakeLists.txt`, add the library you want to use. For example, if you want to add access to elements from `scopy-test` to your plugin, the code will look like this:

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

   By adding the desired library to the `target_link_libraries` section, you can access its functionality in your plugin.

   Ensure that the library you are linking is available and properly configured in your Scopy environment.

9. **Useful Tools**
--------------------------------------------------------------------------------

   We have a couple of tools designed to make some of your work easier. Among these tools are a script for code formatting and a script for license headers. It is recommended that, before making a pull request, you run these two scripts.

   ### License Header Script

   This script should be run when creating new files to ensure they include the appropriate license headers. To run it, use the following command:

   .. code-block:: bash

       ./tools/licence.sh

   ### Formatting Script

   The formatting script ensures that your code follows our coding guidelines. To run it, use the following command:

   .. code-block:: bash

       ./tools/format.sh

10. **IIOWidgets**
--------------------------------------------------------------------------------

   `IIOWidgets` are modular UI components in Scopy designed to facilitate interaction with IIO (Industrial I/O) devices. They provide a graphical interface for displaying and managing data from these devices, making it easier for users to interact with hardware.

   **Key Features of IIOWidgets**

   - **Data Interaction**:
     `IIOWidgets` use data strategies (`DataStrategyInterface`) to handle reading and writing data from IIO devices. They support asynchronous operations such as `readAsync()` and `writeAsync()` to ensure smooth data handling without blocking the UI.

   - **UI Customization**:
     `IIOWidgets` use UI strategies (`GuiStrategyInterface`) to define their appearance and behavior. This allows developers to create widgets that are tailored to specific use cases or device requirements.

   - **Integration with Scopy**:
     `IIOWidgets` are seamlessly integrated into the Scopy application, allowing them to be used alongside other tools and plugins. They can be added to the interface using the `IIOWidgetBuilder`.

   - **Progress and Status Indicators**:
     Many `IIOWidgets` include built-in progress bars and status indicators to provide real-time feedback to users during operations.

   - **Resource Management**:
     Developers can customize `IIOWidgets` with titles, attributes, and other parameters using the `IIOWidgetBuilder`. This ensures that widgets are properly configured for their intended purpose.

   **Creating an IIOWidget**

   Below are two examples of creating `IIOWidgets` using the `IIOWidgetBuilder`:

   **1. Creating a Single IIOWidget for a Specific Device Attribute**

   To create a single `IIOWidget` for a specific device attribute, you can use the following code:

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

   To create all `IIOWidgets` for a specific channel of a device, you can use the following code:

   .. code-block:: cpp

       QList<IIOWidget *> channelWidgets = 
           IIOWidgetBuilder(parent)
               .channel(myChannel)
               .buildAll();

   In this example:
   - `channel()` specifies the channel of the device for which widgets are created.
   - `buildAll()` automatically creates widgets for all attributes of the specified channel.

   **UI Strategy Assignment**

   - When using `buildAll()`, the UI strategy for each widget is assigned automatically based on the attribute type and metadata.
   - When using `buildSingle()`, you can manually specify the UI strategy using the `uiStrategy()` method.

   By leveraging these methods, developers can create both individual and grouped `IIOWidgets` to suit their specific use cases.

   **Benefits of Using IIOWidgets**

   - Simplifies the development of user interfaces for IIO devices.
   - Provides a consistent look and feel across different plugins.
   - Reduces the amount of boilerplate code needed to create UI components.
   - Supports advanced features like asynchronous data handling and progress indicators.

   By leveraging `IIOWidgets`, developers can create powerful and user-friendly interfaces for interacting with IIO devices in Scopy.

11. **Menu UI Items**
--------------------------------------------------------------------------------

   Scopy provides a variety of `Menu` UI components to create intuitive and organized user interfaces. These components are designed to help developers build menus, settings panels, and other hierarchical UI structures with ease. Below is a detailed description of the available `Menu` components and how they can be used.

   **1. MenuSectionWidget**

   - **Purpose**: 
     `MenuSectionWidget` is a container widget used to group related menu items. It provides a layout for organizing multiple widgets, such as buttons, sliders, or dropdowns, within a single section.

   - **Key Features**:
     - Acts as a container for other widgets.
     - Provides a clean and organized structure for menus.
     - Can be used as a standalone section or within collapsible sections.

   - **Example Usage**:
     ```cpp
     MenuSectionWidget *section = new MenuSectionWidget(parent);
     QVBoxLayout *layout = section->contentLayout();
     layout->addWidget(new QLabel("Section Title"));
     layout->addWidget(menuCombo); // Add a MenuCombo widget
     ```

   **2. MenuCollapseSection**

   - **Purpose**: 
     `MenuCollapseSection` is a collapsible container widget that can expand or collapse to show or hide its content. It is ideal for organizing menus with multiple sections, allowing users to focus on specific parts.

   - **Key Features**:
     - Provides expand/collapse functionality.
     - Includes a header with a title and an arrow indicator.
     - Can contain any type of widgets, including `MenuSectionWidget`.

   - **Example Usage**:
     ```cpp
     MenuCollapseSection *collapseSection = new MenuCollapseSection(
         "Collapsible Section", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);
     collapseSection->contentLayout()->addWidget(section); // Add a MenuSectionWidget
     ```

   **3. MenuCombo**

   - **Purpose**: 
     `MenuCombo` is a dropdown menu that allows users to select an option from a list. It combines a label and a `QComboBox` for user interaction.

   - **Key Features**:
     - Provides a labeled dropdown menu.
     - Allows dynamic addition of options.
     - Emits signals when the selected option changes.

   - **Example Usage**:
     ```cpp
     MenuCombo *menuCombo = new MenuCombo("Select Option", parent);
     menuCombo->combo()->addItem("Option 1");
     menuCombo->combo()->addItem("Option 2");
     connect(menuCombo->combo(), &QComboBox::currentTextChanged, this, [](const QString &text) {
         qDebug() << "Selected option:" << text;
     });
     ```

   **4. MenuSpinBox**

   - **Purpose**: 
     `MenuSpinBox` is a numeric input widget that allows users to adjust values within a defined range. It combines a label and a `QSpinBox` for user interaction.

   - **Key Features**:
     - Provides a labeled numeric input field.
     - Supports range and step size configuration.
     - Emits signals when the value changes.

   - **Example Usage**:
     ```cpp
     MenuSpinBox *menuSpinBox = new MenuSpinBox("Set Frequency (Hz)", parent);
     menuSpinBox->spinBox()->setRange(100, 10000); // Set range from 100 Hz to 10,000 Hz
     menuSpinBox->spinBox()->setSingleStep(100);  // Set step size to 100 Hz
     menuSpinBox->spinBox()->setValue(1000);      // Set default value to 1,000 Hz

     connect(menuSpinBox->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this, [](int value) {
         qDebug() << "Frequency set to:" << value << "Hz";
     });
     ```

   **5. MenuHeaderWidget**

   - **Purpose**: 
     `MenuHeaderWidget` is a header widget used to display a title or provide controls like toggle buttons for a menu or section.

   - **Key Features**:
     - Adds a visual header to menus or sections.
     - Can include additional controls, such as buttons or checkboxes.

   - **Example Usage**:
     ```cpp
     MenuHeaderWidget *header = new MenuHeaderWidget("Menu Header", QPen(Qt::black), parent);
     ```

   **6. Creating a Simple Menu with Menu Components**

   Below is an example of how to use these components to create a simple menu:

   ```cpp
   QWidget *createSimpleMenu(QWidget *parent) {
       // Create the main menu container
       QWidget *menu = new QWidget(parent);
       QVBoxLayout *menuLayout = new QVBoxLayout(menu);
       menuLayout->setMargin(0);
       menuLayout->setSpacing(10);

       // Add a header
       MenuHeaderWidget *header = new MenuHeaderWidget("Settings Menu", QPen(Qt::blue), menu);
       menuLayout->addWidget(header);

       // Create a collapsible section
       MenuCollapseSection *collapseSection = new MenuCollapseSection(
           "Frequency Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, menu);

       // Add a MenuSpinBox to the collapsible section
       MenuSpinBox *menuSpinBox = new MenuSpinBox("Set Frequency (Hz)", collapseSection);
       menuSpinBox->spinBox()->setRange(100, 10000);
       menuSpinBox->spinBox()->setSingleStep(100);
       menuSpinBox->spinBox()->setValue(1000);

       connect(menuSpinBox->spinBox(), QOverload<int>::of(&QSpinBox::valueChanged), this, [](int value) {
           qDebug() << "Frequency set to:" << value << "Hz";
       });

       collapseSection->contentLayout()->addWidget(menuSpinBox);

       // Add the collapsible section to the menu
       menuLayout->addWidget(collapseSection);

       // Add a spacer to push content to the top
       menuLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

       return menu;
   }
   ````

   By following these steps, you can create powerful and user-friendly interfaces for interacting with IIO devices in Scopy.

Conclusion
--------------------------------------------------------------------------------
By following these chapters, you will gain a comprehensive understanding of how to create, 
customize, and deploy plugins in Scopy. For more details, refer to the :ref:`plugin_development_guide`.




