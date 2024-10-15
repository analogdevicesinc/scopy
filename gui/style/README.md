# Style tutorial
**Style previewer tool is located in the test plugin**

## Create new style property

Navigate to style/qss/properties and pick/create a folder which matches the widget type you want to create a property for. Inside the folder create a .qss file.
- all style code must be surrounded by {} and have set a property "*[&&property&&=true]{ }". Do not replace the property name, keep it "&&property&&"
- property value can be changed from "true", but when using it in application you also need to specify the correct value
- use keys from the json files as much as possible. example: &unit_1&

Run CMake and the new property should appear in style_properties.h with the same name as the .qss file.

## Add json value
Json files should rarely be altered in any way.
global value: add new key and value in style/json/global.json
theme value: add new key and value in style/json/dark.json and all other theme jsons accordingly
Run CMake and you should find the new value in style_attributes.h.

## Add theme
Add new .json file in style/json.
- add at least all the values from dark.json
- other values from global.json can be overwritten in the new theme. example: "unit_1": "20" (where "unit_1":"16" is in global.json )
- properties can also be overwritten. example: "qss_properties_button_borderButton": "qss_properties_button_basicButton"

## Add plugin style
Create a style folder in your plugin and use the exact same file structure. Only the qss folder is relevant since the json files cannot be overwritten from other plugins.
In your plugin CMakeLists.txt file add the generate_style command.
arguments: **option, style_folder, header_folder** (option should always be "--plugin")

example:
**include(ScopyStyle)
generate_style("--plugin" ${CMAKE_CURRENT_SOURCE_DIR}/style ${CMAKE_CURRENT_SOURCE_DIR}/include/pluginName)**

Run CMake and you should see the new properties in header_folder/style_properties.h.

