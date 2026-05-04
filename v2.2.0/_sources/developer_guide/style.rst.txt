.. _style:

Style Tutorial
==============

**Style previewer tool is located in the test plugin**

Create New Style Property
-------------------------

Before creating a new style, check if you can use an already existing one or combine multiple existing styles. Only create a new one if necessary.

Navigate to ``style/qss/properties`` and pick/create a folder that matches the widget type you want to create a property for. Inside the folder, create a ``.qss`` file.

- All style code must be surrounded by ``{}`` and have a set property ``*[&&property&&=true]{ }``. Do not replace the property name; keep it as ``&&property&&``.
- The property value can be changed from ``true``, but when using it in the application, you also need to specify the correct value.
- Use keys from the JSON files as much as possible. Example: ``&unit_1&``.

Run CMake, and the new property should appear in ``style_properties.h`` with the same name as the ``.qss`` file.

Add JSON Value
--------------

JSON files should rarely be altered in any way.

- **Global value:** Add a new key and value in ``style/json/global.json``.
- **Theme value:** Add a new key and value in ``style/json/dark.json`` and all other theme JSON files accordingly.

Run CMake, and you should find the new value in ``style_attributes.h``.

Add Theme
---------

Add a new ``.json`` file in ``style/json``.

- Add at least all the values from ``dark.json``.
- Other values from ``global.json`` can be overwritten in the new theme. Example:
     "unit_1": "20"  (where "unit_1":"16" is in global.json)

- Properties can also be overwritten. Example:
     "qss_properties_button_borderButton": "qss_properties_button_basicButton"

Add Plugin Style
----------------

Create a ``style`` folder in your plugin and use the exact same file structure. Only the ``qss`` folder is relevant since the JSON files cannot be overwritten from other plugins. You may add a folder in your ``style/qss/properties`` folder with the name of your plugin to avoid overwriting other core ``.qss`` style files with the same name.

In your plugin ``CMakeLists.txt`` file, add the ``generate_style`` command.

**Arguments:** ``option, style_folder, header_folder`` (option should always be ``--plugin``).

Example:

.. code-block:: cmake

   include(ScopyStyle)
   generate_style("--plugin" ${CMAKE_CURRENT_SOURCE_DIR}/style ${CMAKE_CURRENT_SOURCE_DIR}/include/pluginName)

Run CMake, and you should see the new properties in ``header_folder/style_properties.h``.


