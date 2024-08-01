import os
import sys
import shutil
from distutils.dir_util import copy_tree
from typing import Dict, Optional, Set, List


# Creates a namespace structure from files with a given extension in a specified path
def create_namespace_structure(extension: str, path: str, root_namespace: str = ".") -> Dict[str, Dict[str, str]]:
    namespace_structure: Dict[str, Dict[str, str]] = {}

    if os.path.isfile(path):
        namespace_structure[root_namespace] = {
            os.path.splitext(path)[0]: path.replace('\\', '/')
        }
        return namespace_structure

    for dirpath, _, filenames in os.walk(path):
        if len(filenames) == 0:
            continue

        files = [f for f in filenames if f.endswith(extension)]
        if files:
            relative_path = os.path.relpath(dirpath, path)
            if relative_path == ".":
                relative_path = root_namespace
            namespace_structure[relative_path] = {
                os.path.splitext(f)[0]: os.path.join(dirpath, f).replace('\\', '/')
                for f in files
            }

    return namespace_structure


# Replaces a placeholder property in a file with a given value
def replace_property(in_file: str, value: str) -> bool:
    out_file = os.path.join(build_qss_folder, value + ".qss")
    search_text = "&&property&&"

    with open(in_file, 'r') as file:
        data = file.read().replace(search_text, value)

    with open(out_file, 'a') as file:
        file.write(data)

    return True


# Generates a QSS variable for a given file
def generate_qss_variable(filepath: str, indent: str, root_folder: str) -> str:
    value = os.path.relpath(filepath, root_folder).replace("/", "_").replace(".qss", "")
    if not replace_property(filepath, value):
        return ""

    return f'{indent}const char *const ' + os.path.basename(filepath).replace(".qss", '') + f' = "{value}";'


# Generates namespace code based on the given structure
def generate_namespace_code(namespace_structure: Dict[str, Dict[str, str]], variable_func, root_folder: str) -> str:
    code_lines: List[str] = []

    def add_namespaces(folder: List[str], files: Dict[str, str], level: int) -> None:
        indent = '\t' * level
        code_lines.append(f'{indent}namespace {folder[level]} {{')
        if level + 1 == len(folder):
            for filename, filepath in files.items():
                code_lines.append(variable_func(filepath, f'{indent}\t', root_folder))
        else:
            subnamespace = {k: v for k, v in namespace_structure.items() if
                            k.startswith('/'.join(folder[:level + 1]) + '/')}
            if subnamespace:
                for subpath, subfiles in subnamespace.items():
                    subpath_parts = subpath.split(os.sep)
                    add_namespaces(subpath_parts, subfiles, level + 1)
        code_lines.append(f'{indent}}} // namespace {folder[level]}')

    root_parts = sorted(namespace_structure.keys(), key=lambda x: x.count(os.sep))
    added_namespaces: Set[str] = set()

    for root_path in root_parts:
        path_parts = root_path.split(os.sep)
        if path_parts[0] not in added_namespaces:
            if len(namespace_structure[root_path]) != 0:
                if path_parts[0] == GENERAL_FOLDER:
                    # for filename, filepath in namespace_structure[root_path].items():
                    #     variable_func(filepath, '', root_folder)
                    continue
                add_namespaces(path_parts, namespace_structure[root_path], 0)
                added_namespaces.add(path_parts[0])

    return '\n'.join(code_lines)


# Writes the generated namespace code to a header file
def write_header_file(output_file: str, namespace_code: str, name: str, namespace_name: str) -> None:
    if len(namespace_code.strip()) == 0:
        return

    def_name = name.upper().replace(".", "_")
    with open(output_file, 'w') as f:
        f.write("#ifndef " + def_name + "\n")
        f.write("#define " + def_name + "\n")
        f.write("#include <scopy-gui_export.h>\n\n")
        f.write("namespace " + namespace_name + " {\n")
        f.write(namespace_code)
        f.write("\n} // namespace " + namespace_name + "\n")
        f.write("\n\n#endif // " + def_name + "\n")


# Opens the QSS file for writing
def clear_qss_folder(qss_path: str) -> None:
    os.makedirs(qss_path, exist_ok=True)
    for filename in os.listdir(qss_path):
        file_path = os.path.join(qss_path, filename)
        try:
            if os.path.isfile(file_path) or os.path.islink(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)
        except Exception as e:
            print('Failed to delete %s. Reason: %s' % (file_path, e))


# Parses a JSON file and returns a dictionary of its contents
def parse_json_file(filepath: str, root_folder: str) -> Dict[str, str]:
    with open(filepath, 'r') as file:
        content = file.read().strip()

    if content[0] != '{' or content[-1] != '}':
        raise ValueError(f"Invalid JSON format in file {filepath}")

    json_content: Dict[str, str] = {}
    content = content[1:-1].strip()  # Remove the outer braces
    items = content.split(',')

    for item in items:
        key, value = item.split(':', 1)
        key = key.strip().strip('"')
        formatted_key = os.path.relpath(filepath, root_folder).replace("/", "_").replace(".json", "") + "_" + key
        json_content[key] = formatted_key

    return json_content


# Generates JSON variable declarations for a given file
def generate_json_variable(filepath: str, indent: str, root_folder: str) -> str:
    with open(filepath, 'r') as file:
        if len(file.read().strip()) == 0:
            return ''

    json = parse_json_file(filepath, root_folder)
    variable = ""

    for key in json:
        variable += indent + 'const char *const ' + key + ' = "' + key + '";\n'

    return variable[:-1]


# Opens the JSON file for writing with a given start character
def open_json_file(json_path: str) -> None:
    with open(json_path, 'w') as file:
        file.write("{\n")


# Closes the JSON file, ensuring the last comma is removed
def close_json_file(json_path: str) -> None:
    with open(json_path, 'r+') as file:
        content = file.read()
        last_comma_index = content.rfind(',')
        if last_comma_index != -1:
            content = content[:last_comma_index]
            file.seek(0)
            file.write(content)
            file.truncate()
        file.write("\n}\n")


def generate_qss(has_general_style: bool) -> None:
    qss_header_name = "style_properties.h"
    qss_header_path = os.path.join(generated_header_folder, qss_header_name)

    # generate general qss file (only for core use)
    if has_general_style:
        clear_qss_folder(build_qss_folder)
        copy_tree(os.path.join(style_folder, "qss", GENERAL_FOLDER), build_qss_folder)

    # generate header and copy property files to build_qss_folder
    qss_namespace_code = generate_namespace_code(
        create_namespace_structure("qss", os.path.join(style_folder, "qss")),
        generate_qss_variable,
        style_folder,
    )
    write_header_file(qss_header_path, qss_namespace_code, qss_header_name, "style")


def generate_json() -> None:
    json_header_name = "style_attributes.h"
    json_header_path = os.path.join(generated_header_folder, json_header_name)
    global_json_path = os.path.join(style_folder, "json", "global.json")
    theme_json_path = os.path.join(style_folder, "json", "dark.json")

    json_namespace_code = generate_namespace_code(
        create_namespace_structure(".json", global_json_path, "global"),
        generate_json_variable,
        style_folder,
    )
    json_namespace_code += "\n" + generate_namespace_code(
        create_namespace_structure(".json", theme_json_path, "theme"),
        generate_json_variable,
        style_folder,
    )
    write_header_file(json_header_path, json_namespace_code, json_header_name, "json")


def copy_and_append_jsons(src_dir: str, dest_dir: str, append: bool) -> None:
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)

    for root, _, files in os.walk(src_dir):
        for file in files:
            src_file_path = os.path.join(root, file)
            dest_file_path = os.path.join(dest_dir, file)

            with open(src_file_path, 'r') as src_file:
                src_content = src_file.read()

            if os.path.exists(dest_file_path) and append:
                with open(dest_file_path, 'r') as dest_file:
                    dest_content = dest_file.read()
                with open(dest_file_path, 'w') as dest_file:
                    dest_file.write(
                        dest_content[:dest_content.find("\n}")] + ",\n" + src_content[src_content.find("{\n") + 2:])
            else:
                with open(dest_file_path, 'w') as dest_file:
                    dest_file.write(src_content)


"""
options:
- copy json files, generate qss and headers (for core)
    arguments: --core <gui style folder> <gui headers folder> <build folder>
    
- generate qss file and header (for plugins)
    arguments: --plugin <plugin style folder> <plugin headers folder> <build folder>
"""
if __name__ == "__main__":
    if len(sys.argv) != 5:
        raise ValueError(f"Incorrect number of arguments provided. Got {len(sys.argv)} arguments")

    if sys.argv[1] == "--core":
        is_plugin = False
    elif sys.argv[1] == "--plugin":
        is_plugin = True
    else:
        raise ValueError(f"Invalid option. Got {option} and was expecting --core or --plugin")

    GENERAL_FOLDER = 'generic'
    style_folder = sys.argv[2]
    generated_header_folder = sys.argv[3]
    build_folder = sys.argv[4]
    build_qss_folder = os.path.join(build_folder, "style", "qss")
    build_json_folder = os.path.join(build_folder, "style", "json")

    generate_qss(not is_plugin)
    generate_json()
    copy_and_append_jsons(os.path.join(style_folder, "json"), build_json_folder, is_plugin)

    sys.exit(0)
