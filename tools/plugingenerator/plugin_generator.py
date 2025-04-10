#!/bin/python3

import json
import os
import argparse
import shutil
from mako.template import Template

MODE = 0o775
FILES_CREATED = []
DIRS_CREATED = []

def log_created_file(path):
    FILES_CREATED.append(path)

def log_created_dir(path):
    DIRS_CREATED.append(path)

def create_directory(path):
    if not os.path.exists(path):
        os.makedirs(path, MODE)
        log_created_dir(path)
    else:
        print(f"{path} already exists!")

def create_file_from_template(template_path, output_path, **kwargs):
    if not os.path.exists(output_path):
        template = Template(filename=template_path)
        content = template.render(**kwargs)
        with open(output_path, "w") as f:
            f.write(content)
        log_created_file(output_path)
    else:
        print(f"{output_path} already exists!")

def get_args():
    parser = argparse.ArgumentParser(
        prog="Plugin generator",
        description="Support script for plugin development",
    )
    parser.add_argument("-s", "--scopy_path", help="Path to the Scopy repository")
    parser.add_argument("-c", "--config_file_path", help="Path to the configuration file")
    return parser.parse_args()

def load_config(path):
    if not os.path.exists(path):
        print(f"Couldn't find the {path} file!")
        exit(1)
    with open(path, "r") as f:
        return json.load(f)

def confirm(prompt):
    while True:
        answer = input(prompt + " (y/n): ").lower()
        if answer in {"y", "n"}:
            return answer == "y"

def maybe_clear_plugin_dir(plugin_path, config):
    config_path = os.path.join(plugin_path, "plugin_src_config.json")
    if os.path.exists(config_path):
        with open(config_path, "r") as f:
            old_config = json.load(f)
        if old_config != config:
            print("Configuration changed. This will clear existing plugin directory.")
            if confirm("Apply changes and delete existing files?"):
                for item in os.listdir(plugin_path):
                    item_path = os.path.join(plugin_path, item)
                    if os.path.isfile(item_path):
                        os.unlink(item_path)
                    else:
                        shutil.rmtree(item_path)
                return True
            else:
                return False
    return True

def save_config(plugin_path, config):
    config_path = os.path.join(plugin_path, "plugin_src_config.json")
    with open(config_path, "w") as f:
        json.dump(config, f, indent=4)
    log_created_file(config_path)

def generate_tool_sources(tools, include_path, src_path, plugin_name, plugin_export_macro):
    for tool in tools:
        header_path = os.path.join(include_path, f"{tool['file_name']}.h")
        src_path_file = os.path.join(src_path, f"{tool['file_name']}.cpp")

        create_file_from_template(
            "templates/plugin/tool_header_template.mako", header_path,
            plugin_export_macro=plugin_export_macro, plugin_name=plugin_name, config=tool
        )

        create_file_from_template(
            "templates/plugin/tool_src_template.mako", src_path_file,
            config=tool
        )

def generate_plugin(plugin_path, config):
    pdk_enable = config.get("pdk", {}).get("enable", False)
    plugin = config["plugin"]
    test = config.get("test", {})
    resources = config.get("resources", {})
    doc_enabled = config.get("doc", False)

    plugin_name = plugin["plugin_name"]
    plugin_dir = plugin["dir_name"]
    plugin_class = plugin["class_name"]
    plugin_export_macro = f"SCOPY_{plugin_name.upper()}_EXPORT"

    include_path = os.path.join(plugin_path, "include", plugin_name)
    src_path = os.path.join(plugin_path, "src")
    ui_path = os.path.join(plugin_path, "ui")
    doc_path = os.path.join(plugin_path, "doc")
    test_path = os.path.join(plugin_path, "test")
    res_path = os.path.join(plugin_path, "res")

    for path in [include_path, src_path, ui_path]:
        create_directory(path)

    if doc_enabled:
        create_directory(doc_path)

    # Test directory
    if test:
        create_directory(test_path)
        if test["cmakelists"]:
            test_cmake = os.path.join(test_path, "CMakeLists.txt")
            with open(test_cmake, "w") as f:
                f.write(f"cmake_minimum_required(VERSION {test['cmake_min_required']})\n")
                if not pdk_enable:
                    if test.get("tst_pluginloader"):
                        f.write("include(ScopyTest)\n\nsetup_scopy_tests(pluginloader)")
            log_created_file(test_cmake)

        if test.get("tst_pluginloader"):
            plugin_loader_test = os.path.join(test_path, "tst_pluginloader.cpp")
            create_file_from_template(
                "templates/plugin/pluginloader_template.mako", plugin_loader_test,
                tst_classname=f"TST_{plugin_class}", plugin_classname=plugin_class, plugin_name=plugin_name
            )

    # Resources directory
    if resources and resources.get("resources_qrc"):
        create_directory(res_path)
        res_qrc = os.path.join(res_path, "resources.qrc")
        if not os.path.exists(res_qrc):
            with open(res_qrc, "w") as f:
                f.write("<!DOCTYPE RCC>\n<RCC version=\"1.0\"/>")
            log_created_file(res_qrc)

    # Plugin header and source files
    plugin_header = os.path.join(include_path, f"{plugin_name}.h")
    create_file_from_template(
        "templates/plugin/plugin_header_template.mako", plugin_header,
        plugin_export_macro=plugin_export_macro, config=plugin
    )

    plugin_src = os.path.join(src_path, f"{plugin_name}.cpp")
    create_file_from_template(
        "templates/plugin/plugin_src_template.mako", plugin_src,
        config=plugin
    )

    # CMakeLists.txt
    if plugin["cmakelists"]:
        cmake_path = os.path.join(plugin_path, "CMakeLists.txt")
        create_file_from_template(
            "templates/plugin/cmakelists_template.mako", cmake_path,
            pdk_en=False,
            scopy_module=plugin_name,
            plugin_display_name=plugin["plugin_display_name"],
            plugin_description=plugin["plugin_description"],
            config=config["cmakelists"]
        )

    # Cmake config header
    cmakein_path = os.path.join(include_path, f"scopy-{plugin_name}_config.h.cmakein")
    create_file_from_template(
        "templates/plugin/plugin_cmake_config_vars.mako", cmakein_path,
        plugin_name=plugin_name
    )

    generate_tool_sources(plugin["tools"], include_path, src_path, plugin_name, plugin_export_macro)

    # gitignore handling
    gitignore_path = os.path.join(plugin_path, ".gitignore")
    if not os.path.exists(gitignore_path):
        with open(gitignore_path, "w") as f:
            f.write(f"include/{plugin_name}/scopy-{plugin_name}_export.h\n")
            f.write(f"include/{plugin_name}/scopy-{plugin_name}_config.h\n")
        print("Plugin .gitignore file created!")


def generate_pdk(pdkPath, config):
    scopyPdkDeps = config["pdk"]["deps_path"]
    pluginName = config["plugin"]["plugin_name"]
    pluginDirName = config["plugin"]["dir_name"]

    create_directory(pdkPath)

    # preferences.ini
    preferences_path = os.path.join(pdkPath, "preferences.ini")
    if not os.path.exists(preferences_path):
        pdk_preferences = open(preferences_path, "w")
        pdk_preferences.close()

    # PdkSupport.cmake
    create_file_from_template(
        "templates/pdk/pdk_cmake_func_template.mako",
        os.path.join(pdkPath, "PdkSupport.cmake")
    )

    # CMakeLists.txt
    create_file_from_template(
        "templates/pdk/pdk_cmake_template.mako",
        os.path.join(pdkPath, "CMakeLists.txt"),
            deps_path = scopyPdkDeps,
            plugin_dir = pluginDirName,
            plugin_name = pluginName,
            preferences_path = preferences_path
    )

    # include directory and pdkwindow.h + pdk-util_config.h.cmakein
    include_path = os.path.join(pdkPath, "include")
    create_directory(include_path)

    create_file_from_template(
        "templates/pdk/pdk_header_template.mako",
        os.path.join(include_path, "pdkwindow.h")
    )

    create_file_from_template(
        "templates/pdk/pdk_cmakein_template.mako",
        os.path.join(include_path, "pdk-util_config.h.cmakein")
    )

    # src directory and main.cpp
    src_path = os.path.join(pdkPath, "src")
    create_directory(src_path)

    create_file_from_template(
        "templates/pdk/pdk_src_template.mako",
        os.path.join(src_path, "main.cpp")
    )

    # res directory and resources.qrc
    res_path = os.path.join(pdkPath, "res")
    create_directory(res_path)

    res_qrc_path = os.path.join(res_path, "resources.qrc")
    if not os.path.exists(res_qrc_path):
        content = "<RCC>\n <qresource prefix=\"/\">\n </qresource>\n</RCC>"
        with open(res_qrc_path, "w") as f:
            f.write(content)
        FILES_CREATED.append(res_qrc_path)
    else:
        print(res_qrc_path + " file already exists!")
    
    

def main():
    args = get_args()
    scopy_path = args.scopy_path or os.path.dirname(os.path.dirname(os.getcwd()))
    config_path = args.config_file_path or os.path.join(scopy_path, "tools/plugingenerator/config.json")
    config = load_config(config_path)

    # Get the pdk project path from the config
    pdk_project_path = config.get("pdk", {}).get("project_path", "").strip()
    pdk_enable = config.get("pdk", {}).get("enable", False)
    
    
    if pdk_enable:
        # Create the ScopyPluginRunner project if it doesn't exist
        plugin_runner_path = os.path.join(pdk_project_path, "ScopyPluginRunner")
        generate_pdk(plugin_runner_path, config)
        
        # Use the ScopyPluginRunner path as the base for plugin generation
        plugin_path = os.path.join(plugin_runner_path, "plugin", config["plugin"]["dir_name"])
    else:
        # Fall back to the default plugin path under scopy_path
        plugin_path = os.path.join(scopy_path, "plugins", config["plugin"]["dir_name"])

    create_directory(plugin_path)

    if maybe_clear_plugin_dir(plugin_path, config):
        save_config(plugin_path, config)
        generate_plugin(plugin_path, config)

    if DIRS_CREATED:
        print("\nGenerated directories:")
        print(*sorted(DIRS_CREATED), sep="\n")

    if FILES_CREATED:
        print("\nGenerated files:")
        print(*sorted(FILES_CREATED), sep="\n")

    print("\nTo enable building the plugin, add the following to the plugins' CMakeLists.txt:\n")
    plugin_name = config["plugin"]["plugin_name"]
    print(f"""option(ENABLE_PLUGIN_{plugin_name.upper()} "Enable {plugin_name.upper()} plugin" ON)
if(ENABLE_PLUGIN_{plugin_name.upper()})
    add_subdirectory({config['plugin']['dir_name']})
    list(APPEND PLUGINS ${{PLUGIN_NAME}})
endif()""")


if __name__ == "__main__":
    main()
