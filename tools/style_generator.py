import os
import sys


def create_namespace_structure(extension):
    namespace_structure = {}

    for dirpath, dirnames, filenames in os.walk(style_folder):
        qss_files = [f for f in filenames if f.endswith(extension)]
        if qss_files:
            relative_path = os.path.relpath(dirpath, style_folder)
            namespace_structure[relative_path] = {
                os.path.splitext(f)[0]: os.path.join(dirpath, f).replace('\\', '/')
                for f in qss_files
            }

    return namespace_structure


def replace_property(filepath, value):
    search_text = "&&property&&"

    with open(filepath, 'r') as file:
        data = file.read()
        if search_text not in data:
            return False
        data = data.replace(search_text, value)

    with open(qss_path, 'a') as file:
        file.write(data)

    return True


def generate_qss_variable(filepath, indent):
    value = os.path.relpath(filepath, style_folder).replace("/", "_").replace(".qss", "")
    if not replace_property(filepath, value):
        return ""

    return f'{indent}const char *const ' + os.path.basename(filepath).replace(".qss", '') + f' = "{value}";'


def generate_namespace_code(namespace_structure, variable_func):
    code_lines = []

    def add_namespaces(folder, files, level):
        indent = '\t' * level
        code_lines.append(f'{indent}namespace {folder[level]} {{')
        if level + 1 == len(folder):
            for filename, filepath in files.items():
                code_lines.append(variable_func(filepath, f'{indent}\t'))
        else:
            subnamespace = {k: v for k, v in namespace_structure.items() if
                            k.startswith('/'.join(folder[:level + 1]) + '/')}
            if subnamespace:
                for subpath, subfiles in subnamespace.items():
                    subpath_parts = subpath.split(os.sep)
                    add_namespaces(subpath_parts, subfiles, level + 1)
        code_lines.append(f'{indent}}} // namespace {folder[level]}')

    root_parts = sorted(namespace_structure.keys(), key=lambda x: x.count(os.sep))
    added_namespaces = set()

    for root_path in root_parts:
        path_parts = root_path.split(os.sep)
        if path_parts[0] not in added_namespaces:
            add_namespaces(path_parts, namespace_structure[root_path], 0)
            added_namespaces.add(path_parts[0])

    return '\n'.join(code_lines)


def write_header_file(output_file, namespace_code, name):
    def_name = name.upper().replace(".", "_")

    with open(output_file, 'w') as f:
        f.write("#ifndef " + def_name + "\n")
        f.write("#define " + def_name + "\n")
        f.write("#include <scopy-gui_export.h>\n\n")
        f.write(namespace_code)
        f.write("\n\n#endif // " + def_name + "\n")


def open_qss_file():
    open(qss_path, 'w')

def append_variable(key, value):
    with open(json_path, 'a') as file:
        file.write("\t\"" + key + "\": " + value + ",\n")


def parse_json_file(filepath):
    with open(filepath, 'r') as file:
        content = file.read()

    content = content.strip()
    if content[0] != '{' or content[-1] != '}':
        raise ValueError(f"Invalid JSON format in file {filepath}")

    json_content = {}
    content = content[1:-1].strip()  # Remove the outer braces
    items = content.split(',')

    for item in items:
        key, value = item.split(':')
        key = key.strip().strip('"')
        formatted_key = os.path.relpath(filepath, style_folder).replace("/", "_").replace(".json",
                                                                                          "") + "_" + key
        json_content[key] = formatted_key
        append_variable(formatted_key, value)

    return json_content


def generate_json_variable(filepath, indent):
    with open(filepath, 'r') as file:
        if len(file.read()) == 0:
            return ''

    json = parse_json_file(filepath)

    variable = ""
    for key in json:
        variable += indent + 'const char* ' + key + ' = "' + json[key] + '";\n'

    return variable[:-1]



def open_json_file():
    with open(json_path, 'w') as file:
        file.write("{\n")


def close_json_file():
    with open(json_path, 'r+') as file:
        content = file.read()
        last_comma_index = content.rfind(',')
        if last_comma_index != -1:
            content = content[:last_comma_index]
            file.seek(0)
            file.write(content)
            file.truncate()
        file.write("\n}\n")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Error")
        sys.exit(1)

    source_folder = sys.argv[1]
    build_folder = sys.argv[2]
    style_folder = source_folder + "/gui/style"

    # qss
    qss_header_name = "style_properties.h"
    qss_header_path = source_folder + "/gui/include/gui/" + qss_header_name
    qss_name = "style.qss"
    qss_path = build_folder + "/" + qss_name

    open_qss_file()
    qss_namespace_code = generate_namespace_code(create_namespace_structure("qss"),
                                                 generate_qss_variable)
    write_header_file(qss_header_path, qss_namespace_code, qss_header_name)

    # json
    json_header_name = "style_attributes.h"
    json_header_path = source_folder + "/gui/include/gui/" + json_header_name
    json_name = "style_attributes.json"
    json_path = build_folder + "/" + json_name

    open_json_file()
    json_namespace_code = generate_namespace_code(create_namespace_structure(".json"),
                                                  generate_json_variable)
    write_header_file(json_header_path, json_namespace_code, json_header_name)
    close_json_file()

    sys.exit(0)
