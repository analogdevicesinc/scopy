import json
import os
import argparse
import shutil
from mako.template import Template

parser = argparse.ArgumentParser(
    prog="Plugin generator",
    description="Support script for plugin development",
    epilog="Text at the bottom of help",
)

parser.add_argument(
    "-s", "--scopy_path", help="Path to the Scopy repository"
)
parser.add_argument(
    "-c",
    "--config_file_path",
    help="Path to the plugin generator configuration file",
)

args = parser.parse_args()
pathToScopy = ""
if args.scopy_path:
    pathToScopy = args.scopy_path
else:
    pathToScopy = os.path.dirname(os.path.dirname(os.getcwd()))
    
pluginsPath = os.path.join(pathToScopy, "plugins")
if not os.path.exists(pluginsPath):
    print("Couldn't find " + pluginsPath + " path!")
    exit(1)

pathToConfigFile = ""
if args.config_file_path:
    pathToConfigFile = args.config_file_path
else:
    pathToConfigFile = os.path.join(pathToScopy, "tools/plugingenerator/config.json")

if not os.path.exists(pathToConfigFile):
    print("Couldn't find the " + pathToConfigFile + " file!")
    exit(1)

configFile = open(pathToConfigFile, "r")
generatorOptions = json.load(configFile)
configFile.close()

mode = 0o775

filesGenerated = []
directoriesGenerated = []
pluginDirName = generatorOptions["plugin"]["dir_name"]
pluginName = generatorOptions["plugin"]["plugin_name"]
pluginClassName = generatorOptions["plugin"]["class_name"]
pluginExportMacro = "SCOPY_" + pluginName.upper() + "_EXPORT"

print("Starting file generation:")
#################################################### Plugin dir #############################################
newPluginPath = os.path.join(pluginsPath, pluginDirName)
if not os.path.exists(newPluginPath):
    os.mkdir(newPluginPath, mode)
    directoriesGenerated.append(newPluginPath)

############################################# Plugin Source Configuration ####################################
if os.path.exists(newPluginPath):
    pluginSrcConfigPath = os.path.join(newPluginPath, "plugin_src_config.json")
    createSrcJson = True
    if os.path.exists(pluginSrcConfigPath):
        pluginSrcConfigFile = open(pluginSrcConfigPath, "r")
        srcGeneratorOptions = json.load(pluginSrcConfigFile)
        pluginSrcConfigFile.close()
        if srcGeneratorOptions == generatorOptions:
            createSrcJson = False
        else:
            print(
                "The configuration file has been changed! Do you want to apply these changes?\n"
                "*All the data in the plugin forlder will be lost!"
            )
            key = ""
            while key not in {"y", "n"}:
                key = input("Type 'y' (yes) or 'n' (no)\n")
            if key == "y":
                for file in os.listdir(newPluginPath):
                    filePath = os.path.join(newPluginPath, file)
                    if os.path.isfile(filePath):
                        os.unlink(filePath)
                    elif os.path.isdir(filePath):
                        shutil.rmtree(filePath)
            else:
                createSrcJson = False
    if createSrcJson:
        pluginSrcConfigFile = open(pluginSrcConfigPath, "w")
        json.dump(generatorOptions, pluginSrcConfigFile, indent=4)
        pluginSrcConfigFile.close()
        filesGenerated.append(pluginSrcConfigPath)
    else:
        print(pluginSrcConfigPath + " file already exists!")

##################################################### Include ################################################
includePath = os.path.join(newPluginPath, "include")
try:
    os.mkdir(includePath, mode)
    directoriesGenerated.append(includePath)
except FileExistsError:
    print(includePath + " directory already exists!")
pluginIncludePath = os.path.join(includePath, pluginName)
try:
    os.mkdir(pluginIncludePath, mode)
    directoriesGenerated.append(pluginIncludePath)
except FileExistsError:
    print(pluginIncludePath + " directory already exists!")


if os.path.exists(pluginIncludePath):
    pluginHeaderFilePath = os.path.join(pluginIncludePath, pluginName + ".h")
    if not os.path.exists(pluginHeaderFilePath):
        pluginHeaderTemplate = Template(
            filename="templates/plugin_header_template.mako"
        )
        headerContent = pluginHeaderTemplate.render(
            plugin_export_macro=pluginExportMacro, config=generatorOptions["plugin"]
        )
        pluginHeaderFile = open(pluginHeaderFilePath, "w")
        pluginHeaderFile.write(headerContent)
        pluginHeaderFile.close()
        filesGenerated.append(pluginHeaderFilePath)
    else:
        print(pluginHeaderFilePath + " file already exists!")

##################################################### Source #################################################
srcPath = os.path.join(newPluginPath, "src")
try:
    os.mkdir(srcPath, mode)
    directoriesGenerated.append(srcPath)
except FileExistsError:
    print(srcPath + " directory already exists!")

####################################################### Ui ###################################################
try:
    uiPath = os.path.join(newPluginPath, "ui")
    os.mkdir(uiPath, mode)
    directoriesGenerated.append(uiPath)
except FileExistsError:
    print(uiPath + " directory already exists!")

####################################################### Doc ###################################################
if "doc" in generatorOptions and generatorOptions["doc"]:
    docPath = os.path.join(newPluginPath, "doc")
    try:
        os.mkdir(docPath, mode)
        directoriesGenerated.append(docPath)
    except FileExistsError:
        print(docPath + " directory already exists!")

####################################################### Test ###################################################
if "test" in generatorOptions:
    testPath = os.path.join(newPluginPath, "test")
    try:
        os.mkdir(testPath, mode)
        directoriesGenerated.append(testPath)
    except FileExistsError:
        print(testPath + " directory already exists!")
if generatorOptions["test"]["cmakelists"]:
    testCmakePath = os.path.join(testPath, "CMakeLists.txt")
    if not os.path.exists(testCmakePath):
        testCmakeFile = open(testCmakePath, "w")
        testCmakeFile.write(
            "cmake_minimum_required(VERSION "
            + str(generatorOptions["test"]["cmake_min_required"])
            + ")\n\n"
        )
        if generatorOptions["test"]["tst_pluginloader"]:
            testCmakeFile.write("include(ScopyTest)\n\nsetup_scopy_tests(pluginloader)")
        filesGenerated.append(testCmakePath)
    else:
        print(testCmakePath + " file already exists!")
    if generatorOptions["test"]["tst_pluginloader"]:
        pluginLoaderTestPath = os.path.join(testPath, "tst_pluginloader.cpp")
        if not os.path.exists(pluginLoaderTestPath):
            pluginloaderTemplate = Template(
                filename="templates/pluginloader_template.mako"
            )
            pluginLoaderTestContent = pluginloaderTemplate.render(
                tst_classname="TST_" + pluginClassName,
                plugin_classname=pluginClassName,
                plugin_name=pluginName,
            )
            pluginLoaderTestFile = open(pluginLoaderTestPath, "w")
            pluginLoaderTestFile.write(pluginLoaderTestContent)
            pluginLoaderTestFile.close()
            filesGenerated.append(pluginLoaderTestPath)
        else:
            print(pluginLoaderTestPath + " file already exists!")

##################################################### Resources ################################################
if "resources" in generatorOptions:
    resPath = os.path.join(newPluginPath, "res")
    try:
        os.mkdir(resPath, mode)
        directoriesGenerated.append(resPath)
    except FileExistsError:
        print(resPath + " directory already exists!")
    if generatorOptions["resources"]["resources_qrc"]:
        resQrc = os.path.join(resPath, "resources.qrc")
        if not os.path.exists(resQrc):
            resFile = open(resQrc, "w")
            resFile.write("<!DOCTYPE RCC>\n")
            resFile.write('<RCC version="1.0"/>')
            resFile.close()
            filesGenerated.append(resQrc)
        else:
            print(resQrc + " file already exists!")

##################################################### Plugin CMakeLists #########################################
if generatorOptions["plugin"]["cmakelists"]:
    cmakeListsPath = os.path.join(newPluginPath, "CMakeLists.txt")
    cmakeTemplate = Template(filename="templates/cmakelists_template.mako")
    cmakeContent = cmakeTemplate.render(
        scopy_module=pluginName, config=generatorOptions["cmakelists"]
    )

if not os.path.exists(cmakeListsPath):
    cmakeListsFile = open(cmakeListsPath, "w")
    cmakeListsFile.write(cmakeContent)
    cmakeListsFile.close()
    filesGenerated.append(cmakeListsPath)
else:
    print(cmakeListsPath + " file already exists!")

##################################################### Plugin ToolList #########################################
toolList = generatorOptions["plugin"]["tools"]
if toolList:
    for t in toolList:
        if os.path.exists(pluginIncludePath) and os.path.exists(srcPath):
            includeFilePath = os.path.join(pluginIncludePath, t["file_name"] + ".h")
            srcFilePath = os.path.join(srcPath, t["file_name"] + ".cpp")

            includeFileTemplate = Template(
                filename="templates/tool_header_template.mako"
            )
            includeFileContent = includeFileTemplate.render(
                plugin_export_macro=pluginExportMacro, plugin_name=pluginName, config=t
            )
            if not os.path.exists(includeFilePath):
                includeFile = open(includeFilePath, "w")
                includeFile.write(includeFileContent)
                includeFile.close()
                filesGenerated.append(includeFilePath)
            else:
                print(includeFilePath + " file already exists!")

            srcFileTemplate = Template(filename="templates/tool_src_template.mako")
            srcFileContent = srcFileTemplate.render(config=t)

            if not os.path.exists(srcFilePath):
                srcFile = open(srcFilePath, "w")
                srcFile.write(srcFileContent)
                srcFile.close()
                filesGenerated.append(srcFilePath)
            else:
                print(srcFilePath + " file already exists!")
################################################### Plugin Src #################################################
if os.path.exists(srcPath):
    pluginSourceFilePath = os.path.join(srcPath, pluginName + ".cpp")
    if not os.path.exists(pluginSourceFilePath):
        pluginSrcTemplate = Template(filename="templates/plugin_src_template.mako")
        pluginSrcContent = pluginSrcTemplate.render(config=generatorOptions["plugin"])
        pluginSourceFile = open(pluginSourceFilePath, "w")
        pluginSourceFile.write(pluginSrcContent)
        pluginSourceFile.close()
        filesGenerated.append(pluginSourceFilePath)
    else:
        print(pluginSourceFilePath + " file already exists!")

if len(directoriesGenerated) > 0:   
    directoriesGenerated.sort()
    print("\nGenerated directories:")
    print(*directoriesGenerated, sep="\n")

if len(filesGenerated) > 0:   
    filesGenerated.sort()
    print("\nGenerated files:")
    print(*filesGenerated, sep="\n")

pluginsCMakeListsOption = """option(ENABLE_PLUGIN_<<upper_plugin_name>> "Enable <<upper_plugin_name>> plugin" ON)
if(ENABLE_PLUGIN_<<upper_plugin_name>>)
	add_subdirectory(<<plugin_dir_name>>)
	list(APPEND PLUGINS ${PLUGIN_NAME})
endif()"""
pluginsCMakeListsOption = pluginsCMakeListsOption.replace(
    "<<upper_plugin_name>>", pluginName.upper()
)
pluginsCMakeListsOption = pluginsCMakeListsOption.replace(
    "<<plugin_dir_name>>", pluginDirName
)
print(
    "\nTo enable building the plugin go to "
    + os.path.join(pluginsPath, "CMakeLists.txt")
    + " and add:"
)
print(pluginsCMakeListsOption)
