import os
import re
import sys

def count_tests_in_rst(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
    test_pattern = re.compile(r'^Test \d+(:| -) ', re.MULTILINE)
    tests = test_pattern.findall(content)
    return len(tests), content

def parse_test_results(content):
    passed = 0
    failed = 0
    skipped = 0
    parse_pattern = re.compile(r'\*\*Result:*\*\*:*\s*(.*?)(?:\n|$)', re.MULTILINE | re.DOTALL)
    parsed_tests = parse_pattern.findall(content)
    for result in parsed_tests:
        if result.strip() == "FAIL":
            failed += 1
        elif result.strip() == "PASS":
            passed += 1
        else:
            skipped += 1
    return passed, failed, skipped

def process_tests_in_folder(folder_path, output_rst_file):
    for root, _, files in os.walk(folder_path):
        for file in files:
            if file.endswith('.rst'):
                file_path = os.path.join(root, file)
                num_tests, content = count_tests_in_rst(file_path)
                passed, failed, skipped = parse_test_results(content)
                output_to_rst_table(output_rst_file, file.split('.')[0], passed, 
                failed, skipped, num_tests)

def output_rst_header(output_rst_file, folder_name):
    with open(output_rst_file, "w") as file:
        file.write(f".. admonition:: Test Report {folder_name}\n\n")
        file.write(f"   .. list-table::\n")
        file.write(f"      :header-rows: 1\n")
        file.write(f"      :widths: 50 30 30 50 50\n\n")
        file.write(f"      * - {folder_name}\n")
        file.write(f"        - PASS\n")
        file.write(f"        - FAIL\n")
        file.write(f"        - SKIP\n")
        file.write(f"        - Total\n")

def output_to_rst_table(output_rst_file, module, passed, failed, skipped, total):
    if total == 0:
        return
    with open(output_rst_file, "a") as file:
        file.write(f"      * - {module}\n")
        file.write(f"        - {passed}\n")
        file.write(f"        - {failed}\n")
        file.write(f"        - {skipped}\n")
        file.write(f"        - {total}\n")

def output_root_docs_include(docs_root_folder, folder_name):
    with open(os.path.join(docs_root_folder, "tests", "test_report.rst"), "a") as file:
        file.write(f".. include:: ../../testing_results/{folder_name}/test_report.rst\n")

def process_directory(directory, docs_root_folder):
    directory = os.path.abspath(directory)
    folders = os.listdir(directory)
    for testing_dir in folders:
        folder_path = os.path.join(directory, testing_dir)
        if os.path.isdir(folder_path):
            output_rst_file = os.path.join(folder_path, "test_report.rst")
            output_root_docs_include(docs_root_folder, testing_dir)
            output_rst_header(output_rst_file, testing_dir)
            process_tests_in_folder(folder_path, output_rst_file)
            print(f'Tests in {testing_dir} were processed.')

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python test_report_generator.py <docs_root_folder> <testing_results_dir>")
        sys.exit(1)

    docs_root_folder = os.path.abspath(sys.argv[1])
    testing_results_folder = sys.argv[2]
    open(os.path.join(docs_root_folder, "tests", "test_report.rst"), "w").close()
    process_directory(testing_results_folder, docs_root_folder)