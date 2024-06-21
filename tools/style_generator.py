import os
import shutil
import sys
from typing import Dict, Optional, Set, List


# Error handling decorator with detailed error messages
def handle_errors(func):
    def wrapper(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except Exception as e:
            print(f"Error in function '{func.__name__}': {e}")
            sys.exit(-1)

    return wrapper


# Creates a namespace structure from files with a given extension in a specified path
@handle_errors
def create_namespace_structure(extension: str, path: str, root_namespace: str = ".") -> Dict[str, Dict[str, str]]:
    namespace_structure: Dict[str, Dict[str, str]] = {}

    if os.path.isfile(path):
        namespace_structure[root_namespace] = {
            os.path.splitext(path)[0]: path.replace('\\', '/')
        }
        return namespace_structure

    try:
        for dirpath, _, filenames in os.walk(path):
            files = [f for f in filenames if f.endswith(extension)]
            if files:
                relative_path = os.path.relpath(dirpath, path)
                if relative_path == ".":
                    relative_path = root_namespace
                namespace_structure[relative_path] = {
                    os.path.splitext(f)[0]: os.path.join(dirpath, f).replace('\\', '/')
                    for f in files
                }
    except Exception as e:
        raise RuntimeError(f"Failed to create namespace structure for path '{path}' with extension '{extension}': {e}")

    return namespace_structure


# Replaces a placeholder property in a file with a given value
@handle_errors
def replace_property(filepath: str, value: str) -> bool:
    search_text = "&&property&&"

    try:
        with open(filepath, 'r') as file:
            data = file.read().replace(search_text, value)

        with open(qss_path, 'a') as file:
            file.write(data)
    except Exception as e:
        raise RuntimeError(f"Failed to replace property in file '{filepath}': {e}")

    return True


# Generates a QSS variable for a given file
@handle_errors
def generate_qss_variable(filepath: str, indent: str, root_folder: str) -> str:
    try:
        value = os.path.relpath(filepath, root_folder).replace("/", "_").replace(".qss", "")
        if not replace_property(filepath, value):
            return ""

        return f'{indent}const char *const ' + os.path.basename(filepath).replace(".qss", '') + f' = "{value}";'
    except Exception as e:
        raise RuntimeError(f"Failed to generate QSS variable for file '{filepath}': {e}")


# Generates namespace code based on the given structure
@handle_errors
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

    try:
        root_parts = sorted(namespace_structure.keys(), key=lambda x: x.count(os.sep))
        added_namespaces: Set[str] = set()

        for root_path in root_parts:
            path_parts = root_path.split(os.sep)
            if path_parts[0] not in added_namespaces:
                add_namespaces(path_parts, namespace_structure[root_path], 0)
                added_namespaces.add(path_parts[0])
    except Exception as e:
        raise RuntimeError(f"Failed to generate namespace code: {e}")

    return '\n'.join(code_lines)


# Writes the generated namespace code to a header file
@handle_errors
def write_header_file(output_file: str, namespace_code: str, name: str, namespace_name: str) -> None:
    def_name = name.upper().replace(".", "_")

    try:
        with open(output_file, 'w') as f:
            f.write("#ifndef " + def_name + "\n")
            f.write("#define " + def_name + "\n")
            f.write("#include <scopy-gui_export.h>\n\n")
            f.write("namespace " + namespace_name + " {\n")
            f.write(namespace_code)
            f.write("\n} // namespace " + namespace_name + "\n")
            f.write("\n\n#endif // " + def_name + "\n")
    except Exception as e:
        raise RuntimeError(f"Failed to write header file '{output_file}': {e}")


# Opens the QSS file for writing
@handle_errors
def open_qss_file() -> None:
    try:
        open(qss_path, 'w')
    except Exception as e:
        raise RuntimeError(f"Failed to open QSS file '{qss_path}' for writing: {e}")


# Appends a variable to a JSON file
@handle_errors
def append_variable(key: str, value: str) -> None:
    if not json_path:
        return

    if json_with_value:
        value = ":" + value
    else:
        value = ""
    try:
        with open(json_path, 'a') as file:
            file.write("\t\"" + key + "\"" + value + ",\n")
    except Exception as e:
        raise RuntimeError(f"Failed to append variable '{key}' to JSON file '{json_path}': {e}")


# Parses a JSON file and returns a dictionary of its contents
@handle_errors
def parse_json_file(filepath: str, root_folder: str) -> Dict[str, str]:
    try:
        with open(filepath, 'r') as file:
            content = file.read().strip()

        if content[0] != '{' or content[-1] != '}':
            raise ValueError(f"Invalid JSON format in file {filepath}")

        json_content: Dict[str, str] = {}
        content = content[1:-1].strip()  # Remove the outer braces
        items = content.split(',')

        for item in items:
            key, value = item.split(':')
            key = key.strip().strip('"')
            formatted_key = os.path.relpath(filepath, root_folder).replace("/", "_").replace(".json", "") + "_" + key
            json_content[key] = formatted_key
            append_variable(formatted_key, value)
    except Exception as e:
        raise RuntimeError(f"Failed to parse JSON file '{filepath}': {e}")

    return json_content


# Generates JSON variable declarations for a given file
@handle_errors
def generate_json_variable(filepath: str, indent: str, root_folder: str) -> str:
    try:
        with open(filepath, 'r') as file:
            if len(file.read()) == 0:
                return ''

        json = parse_json_file(filepath, root_folder)
        variable = ""

        for key in json:
            variable += indent + 'const char *const ' + key + ' = "' + key + '";\n'

        return variable[:-1]
    except Exception as e:
        raise RuntimeError(f"Failed to generate JSON variable for file '{filepath}': {e}")


# Opens the JSON file for writing with a given start character
@handle_errors
def open_json_file(start_char: str = "{") -> None:
    try:
        with open(json_path, 'w') as file:
            file.write(start_char + "\n")
    except Exception as e:
        raise RuntimeError(f"Failed to open JSON file '{json_path}' for writing: {e}")


# Closes the JSON file, ensuring the last comma is removed
@handle_errors
def close_json_file(end_char: str = "}") -> None:
    try:
        with open(json_path, 'r+') as file:
            content = file.read()
            last_comma_index = content.rfind(',')
            if last_comma_index != -1:
                content = content[:last_comma_index]
                file.seek(0)
                file.write(content)
                file.truncate()
            file.write("\n" + end_char + "\n")
    except Exception as e:
        raise RuntimeError(f"Failed to close JSON file '{json_path}': {e}")


# Combines multiple JSON files into one
@handle_errors
def combine_json_files(input_folder: str, output_file: str) -> None:
    combined_data = '['
    first_file = True

    try:
        for root, _, files in os.walk(input_folder):
            for file in files:
                if file.endswith('.json'):
                    with open(os.path.join(root, file), 'r') as f:
                        if not first_file:
                            combined_data += ','
                        combined_data += f.read().strip()
                        first_file = False

        combined_data += ']'

        with open(output_file, 'w') as f:
            f.write(combined_data)
    except Exception as e:
        raise RuntimeError(f"Failed to combine JSON files in '{input_folder}' into '{output_file}': {e}")


if __name__ == "__main__":
    try:
        if len(sys.argv) != 3:
            raise ValueError("Incorrect number of arguments provided")

        source_folder = sys.argv[1]
        build_folder = sys.argv[2]
        style_folder = os.path.join(source_folder, "gui", "style")

        # QSS
        qss_header_name = "style_properties.h"
        qss_header_path = os.path.join(source_folder, "gui", "include", "gui", qss_header_name)
        qss_name = "style.qss"
        qss_path = os.path.join(build_folder, qss_name)

        open_qss_file()
        qss_namespace_code = generate_namespace_code(
            create_namespace_structure("qss", os.path.join(style_folder, "qss")),
            generate_qss_variable,
            style_folder
        )
        write_header_file(qss_header_path, qss_namespace_code, qss_header_name, "style")

        # JSON
        json_header_name = "style_attributes.h"
        json_header_path = os.path.join(source_folder, "gui", "include", "gui", json_header_name)

        global_json_path = os.path.join(style_folder, "json", "global.json")
        theme_json_path = os.path.join(style_folder, "json", "dark.json")

        json_path = os.path.join(build_folder, "style_attributes.json")
        json_with_value = False

        open_json_file()
        json_namespace_code = generate_namespace_code(
            create_namespace_structure(".json", global_json_path, "global"),
            generate_json_variable,
            style_folder
        )
        json_namespace_code += "\n" + generate_namespace_code(
            create_namespace_structure(".json", theme_json_path, "theme"),
            generate_json_variable,
            style_folder
        )
        write_header_file(json_header_path, json_namespace_code, json_header_name, "json")

        combine_json_files(os.path.join(style_folder, "json"), json_path)
        close_json_file()

    except Exception as e:
        print(f"Error: {e}")
        sys.exit(-1)
