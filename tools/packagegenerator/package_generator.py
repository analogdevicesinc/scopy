#!/bin/python3

import json
import os
import argparse
import shutil
from mako.template import Template
import subprocess
import platform

if platform.system() != "Windows":
    MODE = 0o775
else:
    MODE = None
FILES_CREATED = []
DIRS_CREATED = []
REQUIRED_PLUGIN_FIELDS = ["plugin_name", "plugin_display_name", "plugin_description", 
                        "class_name", "namespace", "device_category", "tools", 
                        "cmakelists", "style", "test", "resources", "doc"]
TEMPLATES = {
    "plugin_cmakein": os.path.join("templates","plugin","plugin_cmakein.mako"),
    "plugin_loader": os.path.join("templates", "plugin", "pluginloader.mako"),
    "plugin_header": os.path.join("templates", "plugin", "plugin_header.mako"),
    "plugin_src": os.path.join("templates", "plugin", "plugin_src.mako"),
    "plugin_cmake": os.path.join("templates", "plugin", "cmakelists.mako"),
    "tool_header": os.path.join("templates", "plugin", "tool_header.mako"),
    "tool_src": os.path.join("templates", "plugin", "tool_src.mako"),
    "pkg_cmake": os.path.join("templates", "package", "pkg_cmakelists.mako"),
    "manifest": os.path.join("templates", "package", "manifest_cmakein.mako"),
    "pdk_header": os.path.join("templates", "pdk", "pdk_header.mako"),
    "pdk_src": os.path.join("templates", "pdk", "pdk_src.mako"),
    "pdk_cmake": os.path.join("templates", "pdk", "pdk_cmakelists.mako"),
    "pdk_cmakein": os.path.join("templates", "pdk", "pdk_cmakein.mako"),
    "pdk_cmake_module": os.path.join("templates", "pdk", "pdk_cmake_module.mako")
}

def log_created_file(path):
    FILES_CREATED.append(path)

def log_created_dir(path):
    DIRS_CREATED.append(path)
    
def has_all_required_fields(config, required_fields):
    return all(field in config for field in required_fields)

def is_git_available():
    result = subprocess.run(["git", "--version"], capture_output=True, text=True)
    return result.returncode == 0

def create_directory(path):
    if not os.path.exists(path):
        if MODE:
            os.makedirs(path, MODE)
        else:
            os.makedirs(path)
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

def run_command(command):
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error: {result.stderr}")
        return False
    return True

def get_args():
    parser = argparse.ArgumentParser(
        prog="Package generator",
        description="Support script for package development",
    )
    parser.add_argument("-a", "--archive", help="Archive packages", action='store_true')
    parser.add_argument("-p", "--plugin", help="Generate a plugin (optionally specify the path to the plugin.json file)", 
                        nargs='?',const=True)
    parser.add_argument("-t", "--translation", help="Add translations templates", action='store_true')
    parser.add_argument("--all", help="Generate the entire package structure (plugins, style, translations).", action='store_true')
    parser.add_argument("--add_submodule", help="Add an existing git repository as a submodule")
    parser.add_argument("--style", help="Add style templates", action='store_true')
    parser.add_argument("--init", help="Initialize a new Git submodule in the packages directory of the Scopy repository. "
                        "Ensure the config.json file is properly configured.", action='store_true')
    parser.add_argument("--pdk", help="Generate pdk. The pdk fields from config.json must be completed", action='store_true')
    parser.add_argument("--src", help="Path to the source directory for the archive. If not specified, the current directory will be used")
    parser.add_argument("--dest", help="Path to the destination directory. If not specified, the current directory will be used")
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
            TEMPLATES["tool_header"], header_path,
            plugin_export_macro=plugin_export_macro, plugin_name=plugin_name, config=tool
        )

        create_file_from_template(
            TEMPLATES["tool_src"], src_path_file,
            config=tool
        )

def generate_plugin(plugin_path, plugin, pdk_enable=False):
    """
    Generates a plugin structure with headers, source files, and optional resources.

    Args:
        plugin_path (str): Path to the directory where the plugin will be created.
        plugin (dict): Plugin configuration dictionary containing details like name, class, and resources.
        pdk_enable (bool): Whether the plugin is part of a PDK project (default: False).

    Functionality:
        - Creates directories for headers, source files, UI, and resources.
        - Generates plugin-specific files, including `CMakeLists.txt`, headers, and source files.
        - Handles optional features like styles, tests, and resources.
    """
    if not has_all_required_fields(plugin, REQUIRED_PLUGIN_FIELDS):
        print("Missing required fields in plugin configuration.")
        return
    print("Generating plugin...")
    test = plugin.get("test", {})
    resources = plugin.get("resources", {})
    doc_enabled = plugin.get("doc", False)

    plugin_name = plugin["plugin_name"]
    plugin_class = plugin["class_name"]
    plugin_export_macro = f"SCOPY_{plugin_name.upper()}_EXPORT"

    include_path = os.path.join(plugin_path, "include", plugin_name)
    src_path = os.path.join(plugin_path, "src")
    ui_path = os.path.join(plugin_path, "ui")
    doc_path = os.path.join(plugin_path, "doc")
    test_path = os.path.join(plugin_path, "test")
    res_path = os.path.join(plugin_path, "res")
    style_path = os.path.join(plugin_path, "style")
    style_qss_path = os.path.join(style_path, "qss", "properties", plugin_name)
    style_json_path = os.path.join(style_path, "json")

    for path in [include_path, src_path, ui_path]:
        create_directory(path)

    if plugin.get("style", {}).get("qss", False):
        create_directory(style_qss_path)
        
    if plugin.get("style", {}).get("json", False):
        create_directory(style_json_path)

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
                TEMPLATES["plugin_loader"], plugin_loader_test,
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
        TEMPLATES["plugin_header"], plugin_header,
        plugin_export_macro=plugin_export_macro, plugin_name=plugin_name, config=plugin
    )

    plugin_src = os.path.join(src_path, f"{plugin_name}.cpp")
    create_file_from_template(
        TEMPLATES["plugin_src"], plugin_src, plugin_name=plugin_name,
        config=plugin
    )

    # CMakeLists.txt
    
    cmake_path = os.path.join(plugin_path, "CMakeLists.txt")
    create_file_from_template(
        TEMPLATES["plugin_cmake"], cmake_path,
        pdk_en=pdk_enable,
        author=config["author"],
        license=config["license"],
        category=config["category"],
        scopy_module=plugin["plugin_name"],
        plugin_display_name=plugin["plugin_display_name"],
        plugin_description=plugin["plugin_description"],
        config=plugin["cmakelists"],
        style_qss=plugin.get("style", {}).get("qss", False),
        style_json=plugin.get("style", {}).get("json", False)
    )

    # Cmake config header
    cmakein_path = os.path.join(include_path, f"scopy-{plugin_name}_config.h.cmakein")
    create_file_from_template(
        TEMPLATES["plugin_cmakein"], cmakein_path,
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

def generate_translation(resources_path):
    """
    Adds translation templates to the package.

    Args:
        resources_path (str): Path to the package resources directory.

    Functionality:
        - Creates a `translations` directory and a `translations.qrc` file.
        - Ensures the directory structure is ready for adding translation files.
    """
    print("Adding translation templates...")
    translation_path = os.path.join(resources_path, "translations")
    create_directory(translation_path)
    translations_qrc = os.path.join(resources_path, "translations.qrc")
    if not os.path.exists(translations_qrc):
        with open(translations_qrc, "w") as f:
            f.write("<RCC>\n<qresource prefix=\"/translations\">\n"
                    "@TRANSLATIONS@\n</qresource>\n</RCC>")
        log_created_file(translations_qrc)

def generate_style(pkg_path):
    """
    Adds style templates to the package.

    Args:
        pkg_path (str): Path to the package directory.

    Functionality:
        - Creates directories for styles, including `qss` and `json`.
        - Adds `.gitkeep` files to ensure empty directories are tracked in version control.
    """
    print("Adding style templates...")
    # Create directories for style
    create_directory(os.path.join(pkg_path, "include", os.path.basename(pkg_path)))
    style_path = os.path.join(pkg_path, "style")
    create_directory(style_path)
    style_qss_path = os.path.join(style_path, "qss")
    create_directory(style_qss_path)
    generic_path = os.path.join(style_qss_path, "generic")
    properties_path = os.path.join(style_qss_path, "properties")
    create_directory(generic_path)
    create_directory(properties_path)
    style_json_path = os.path.join(style_path, "json")
    create_directory(style_json_path)
    # Create .gitkeep files (delete if not needed)
    if not os.listdir(generic_path):
        open(os.path.join(generic_path, ".gitkeep"), "w").close()
    if not os.listdir(properties_path):
        open(os.path.join(properties_path, ".gitkeep"), "w").close()
    if not os.listdir(style_json_path):
        open(os.path.join(style_json_path, ".gitkeep"), "w").close()

def generate_pdk(pdkPath, plugin):
    scopyPdkDeps = plugin["pdk"]["deps_path"]
    pluginName = plugin["plugin_name"]
    pluginDirName = plugin["plugin_name"]

    create_directory(pdkPath)

    # preferences.ini
    preferences_path = os.path.join(pdkPath, "preferences.ini")
    if not os.path.exists(preferences_path):
        pdk_preferences = open(preferences_path, "w")
        pdk_preferences.close()

    # PdkSupport.cmake
    create_file_from_template(
        TEMPLATES["pdk_cmake_module"],
        os.path.join(pdkPath, "PdkSupport.cmake")
    )

    # CMakeLists.txt
    create_file_from_template(
        TEMPLATES["pdk_cmake"],
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
        TEMPLATES["pdk_header"],
        os.path.join(include_path, "pdkwindow.h")
    )

    create_file_from_template(
        TEMPLATES["pdk_cmakein"],
        os.path.join(include_path, "pdk-util_config.h.cmakein")
    )

    # src directory and main.cpp
    src_path = os.path.join(pdkPath, "src")
    create_directory(src_path)

    create_file_from_template(
        TEMPLATES["pdk_src"],
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

def generate_manifest(path, config):
    manifest_path = os.path.join(path, "manifest.json.cmakein")
    if os.path.exists(manifest_path):
        os.remove(manifest_path)
    create_file_from_template(
            TEMPLATES["manifest"], manifest_path, config=config)

def handle_cmakelists(pkg_path, config, args):
    cmakelists_path = os.path.join(pkg_path, "CMakeLists.txt")
    new_cmakelists_content = Template(filename=TEMPLATES["pkg_cmake"]).render(
        id=config["id"],
        en_translation=args.translation or args.all,
        en_style=args.style or args.all).splitlines(keepends=True) 

    if os.path.exists(cmakelists_path):
        with open(cmakelists_path, "r") as existing_file:
            existing_content = existing_file.readlines()

        filtered_new_content = [line for line in new_cmakelists_content if line not in existing_content]
        if not filtered_new_content:
            print("No new lines to add. Skipping update.")
            return
        try:
            index = next(i for i, line in enumerate(existing_content) if "Including plugins" in line)
            merged_content = existing_content[:index] + filtered_new_content + existing_content[index:]
        except StopIteration:
            print(f"Warning: 'including plugins' not found in {cmakelists_path}. Appending new content.")
            merged_content = existing_content + ["\n"] + filtered_new_content

        with open(cmakelists_path, "wt") as updated_file:
            updated_file.writelines(merged_content)
        log_created_file(cmakelists_path)

    else:
        create_file_from_template(
            TEMPLATES["pkg_cmake"], 
            cmakelists_path,
            id=config["id"],
            en_translation=args.translation or args.all,
            en_style=args.style or args.all
        )


def generate_pkg(packages_path, config, args):
    """
    Generates a complete package structure, including plugins, translations, and styles.

    Args:
        packages_path (str): Path to the directory where packages will be created.
        config (dict): Configuration dictionary loaded from `pkg.json`.
        args (argparse.Namespace): Parsed command-line arguments.

    Functionality:
        - Creates the package directory and handles the `CMakeLists.txt` file.
        - Generates plugins, translations, and styles based on the provided arguments.
        - Saves the package configuration and generates a `manifest.json.cmakein` file.
    """
    print("Generating package...")
    pkg_path = os.path.join(packages_path, config["id"])
    resources_path = os.path.join(pkg_path, "resources")
    create_directory(pkg_path)
    handle_cmakelists(pkg_path, config, args)
    
    if args.plugin or args.all:
        if isinstance(args.plugin, str):
            plugin_config_path = args.plugin
        else:
            plugin_config_path = os.path.join(scopy_path, "tools/packagegenerator/plugin.json")
        print("Using plugin config file:", plugin_config_path)
        plugin = load_config(plugin_config_path)
        plugins_path = os.path.join(pkg_path, "plugins")
        create_directory(plugins_path)
        plugin_path = os.path.join(plugins_path, plugin["plugin_name"])
        create_directory(plugin_path)
        if maybe_clear_plugin_dir(plugin_path, plugin):
            save_config(plugin_path, plugin)
            generate_plugin(plugin_path, plugin)   
            
    if args.translation or args.all:
        generate_translation(resources_path)
        
    if args.style or args.all:
        generate_style(pkg_path)  
        
    if os.path.exists(pkg_path):
        generate_manifest(pkg_path, config)
        
    
def console_log(config):
    if DIRS_CREATED:
        print("\nGenerated directories:")
        print(*sorted(DIRS_CREATED), sep="\n")

    if FILES_CREATED:
        print("\nGenerated files:")
        print(*sorted(FILES_CREATED), sep="\n")
 
def pdk(config, args):
    """
    Generates a Plugin Development Kit (PDK) project and optionally creates a plugin within it.

    Args:
        config (dict): Configuration dictionary containing the plugin details.
        args (argparse.Namespace): Parsed command-line arguments.

    Functionality:
        - Reads the PDK project path from the plugin configuration.
        - Creates the Scopy Plugin Runner project if it doesn't exist.
        - Generates the PDK structure and optionally a plugin within it.
        - For plugin generation the plugin.json file is used.
    """
    print("Generating PDK...")
    plugin_config_path = os.path.join(scopy_path, "tools/packagegenerator/plugin.json")
    plugin = load_config(plugin_config_path)
    # Get the pdk project path from the config
    pdk_project_path = plugin.get("pdk", {}).get("project_path", "").strip()
    
    if args.pdk and pdk_project_path:
        print("Creating Scopy Plugin Runner project...")
        # Create the ScopyPluginRunner project if it doesn't exist
        plugin_runner_path = os.path.join(pdk_project_path, "ScopyPluginRunner")
        generate_pdk(plugin_runner_path, plugin)
        
        # Use the ScopyPluginRunner path as the base for plugin generation
        plugin_path = os.path.join(plugin_runner_path, "plugin", plugin["plugin_name"])
        create_directory(plugin_path)
        if maybe_clear_plugin_dir(plugin_path, plugin):
            save_config(plugin_path, plugin)
            generate_plugin(plugin_path, plugin, True)
            generate_manifest(plugin_path, config)
    
def archive_package(package_path, destination_dir):
    if not os.path.exists(package_path):
        print(f"Package path {package_path} does not exist.")
        return False
    
    if not os.path.isdir(package_path):
        print(f"{package_path} is not a directory.")
        return False

    manifest_file = os.path.join(package_path, "MANIFEST.json")
    if os.path.exists(manifest_file):
        # Create a zip archive of the package directory
        archive_name = os.path.join(destination_dir, os.path.basename(package_path))
        packages_path = os.path.dirname(package_path)

        archive = shutil.make_archive(archive_name, 'zip', packages_path, os.path.basename(package_path))
        if archive is None:
            print(f"Failed to create archive for {package_path}.")
            return False
        else:
            print(f"Package archived as {archive_name}")
            return True
            
    print(f"Manifest file {manifest_file} does not exist.")
    return False 

def archive_all_packages(packages_path, destination_dir):
    if not os.path.exists(packages_path):
        print(f"Packages path {packages_path} does not exist.")
        return
    
    print("If the provided path is a package, it will be archived. If it is a directory, all packages in the directory will be archived.")
    if archive_package(packages_path, destination_dir):
        return
    
    print(f"Archiving all packages in {packages_path} to {destination_dir}")
    # Create a zip archive for each package in the packages directory
    if not os.path.exists(destination_dir):
        os.makedirs(destination_dir)
    for package_name in os.listdir(packages_path):
        package_path = os.path.join(packages_path, package_name)
        archive_package(package_path, destination_dir) 

def init_submodule_and_generate_pkg(packages_path, config, args):
    """
    Initializes a new Git submodule and generates the package in it.
    
    Args:
        packages_path (str): Path to the packages directory.
        config (dict): Configuration dictionary.
        plugin (bool): Flag indicating whether to generate a plugin.
    """    
    submodule_path = os.path.join(packages_path, config["id"])
    if os.path.exists(submodule_path):
        print(f"Submodule path {submodule_path} already exists.")
    else:
        # Create the submodule directory and initialize a new repository
        create_directory(submodule_path)
        if not run_command(f"git init {submodule_path}"):
            print(f"Git init failed for {submodule_path}.")
            return
        if not run_command(f"git -C {submodule_path} checkout -b main"):
            print(f"Failed to create a new branch at {submodule_path}.")
            return
        print(f"Initialized new git repository at {submodule_path}.")
    
    # Generate the package in the submodule
    generate_pkg(packages_path, config, args)
    
def add_existing_git_submodule(packages_path, repo_url):
    """
    Adds an existing Git repository as a submodule.

    Args:
        packages_path (str): Path to the packages directory.
        repo_url (str): URL of the Git repository to add as a submodule.
    """
    print(f"Adding existing repository {repo_url} as submodule...")
    # Extract repository name from URL
    repo_name = os.path.splitext(os.path.basename(repo_url))[0]
    submodule_path = os.path.join(packages_path, repo_name)

    if os.path.exists(submodule_path):
        print(f"Submodule path {submodule_path} already exists.")
        return

    # Run Git commands
    print("Add submodule...")
    if not run_command(f"git submodule add -b main {repo_url} {submodule_path}"):
        print("Failed to add submodule.")
        return

    print("Update submodule...")
    if not run_command(f"git submodule update --init --recursive {submodule_path}"):
        print("Failed to update submodule.")
        return

    print(f"Added existing repository as submodule at {submodule_path}.")

if __name__ == "__main__":
    args = get_args()
    src_dir =  args.src or os.path.curdir
    dest_dir = args.dest or os.path.curdir
    scopy_path = args.scopy_path or os.path.dirname(os.path.dirname(os.getcwd()))
    # The code seems to be a comment in Python. It starts with a `#` symbol, which indicates that the
    # following text is a comment and will not be executed as code. The text "packages_path" is likely
    # a placeholder or description for the code that follows or for the reader's reference.
    packages_path = os.path.join(scopy_path, "packages")
    config_path = args.config_file_path or os.path.join(scopy_path, "tools/packagegenerator/pkg.json")
    config = load_config(config_path)
    
    if args.archive:
        archive_all_packages(src_dir, dest_dir)
    elif args.pdk:
        pdk(config, args)
        console_log(config)
    elif args.init:
        if not is_git_available():
            print("Git is not installed or not available in PATH.")
            exit(1)
        init_submodule_and_generate_pkg(packages_path, config, args)
        console_log(config)
    elif args.add_submodule:
        if not is_git_available():
            print("Git is not installed or not available in PATH.")
            exit(1)
        add_existing_git_submodule(packages_path, args.add_submodule)
    elif args.plugin or args.style or args.translation:
        generate_pkg(packages_path, config, args)
        console_log(config)
