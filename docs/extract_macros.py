import os
import sys
import re

def split_arguments(args):
    arg_lst = []
    current = []
    inside_string = False
    for char in args:
        if char == '"' and (not current or current[-1] != '\\'):
            inside_string = not inside_string
        if char == ',' and not inside_string:
            arg_lst.append(''.join(current).strip())
            current = []
        else:
            current.append(char)
    arg_lst.append(''.join(current).strip())
    return arg_lst

def parse_file(file_path, macro_name):
    macros = []
    with open(file_path, "r") as file:
        content = file.read()
        match_lst = re.findall(re.escape(macro_name) + r'\s*\((.*?)\)', content, re.DOTALL)
        for match in match_lst:
            args = split_arguments(match)
            if len(args) >= 4 and args[1] != "id":
                id_arg = args[1].replace('"', '')
                title_arg = args[2].replace('"', '')
                description_arg = args[3].replace('"', '')
                description_arg = description_arg.replace('\n', '')
                description_arg = description_arg.replace('\t', '')
                description_arg.strip()
                macros.append((id_arg, title_arg, description_arg))
    return macros

def search_files_in_path(search_path):
    all_macros = []
    macro_pattern = ['PREFERENCE_CHECK_BOX', 'PREFERENCE_EDIT', 'PREFERENCE_COMBO', 'PREFERENCE_COMBO_LIST']
    for root, _, files in os.walk(search_path):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                for macro in macro_pattern:
                    macros_in_file = parse_file(os.path.join(root, file), macro)
                    all_macros.extend(macros_in_file)
    return all_macros

def output_to_rst_table(macros, output_rst_file):
    with open(output_rst_file, "w") as file:
        file.write(".. list-table::\n")
        file.write("   :header-rows: 1\n")
        file.write("   :widths: 20 40 50\n\n")
        file.write("   * - ID\n")
        file.write("     - Title\n")
        file.write("     - Description\n")
        for macro in macros:
            file.write(f"   * - {macro[0]}\n")
            file.write(f"     - {macro[1]}\n")
            file.write(f"     - {macro[2]}\n")

if len(sys.argv) != 3:
    print("python extract_macros.py <search_path> <output_rst_file>")
    sys.exit(1)

search_path = sys.argv[1]
output_rst_file = sys.argv[2]
macros = search_files_in_path(search_path)
output_to_rst_table(macros, output_rst_file)