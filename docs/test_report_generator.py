import os
import re
import sys


def scopy_test_resources_base_dir(version: str) -> str:
    return f"https://github.com/analogdevicesinc/scopy/blob/{version}/testing_results/"


class TestSuiteOverview:
    @staticmethod
    def output_rst_header(output_rst_file: str) -> None:
        with open(output_rst_file, "w") as file:
            file.write(f".. admonition:: Test Report {output_rst_file}\n\n")
            file.write("   .. list-table::\n")
            file.write("      :header-rows: 1\n")
            file.write("      :widths: 50 30 30 50 50 50\n\n")
            file.write("      * - NAME\n")
            file.write("        - PASS\n")
            file.write("        - FAIL\n")
            file.write("        - SKIP\n")
            file.write("        - Total\n")
            file.write("        - Details\n")

    @staticmethod
    def output_to_rst_table(output_rst_file: str, name: str, link: str, passed: int, failed: int, skipped: int,
                            total: int) -> None:
        with open(output_rst_file, "a") as file:
            file.write(f"      * - {name}\n")
            file.write(f"        - {passed}\n")
            file.write(f"        - {failed}\n")
            file.write(f"        - {skipped}\n")
            file.write(f"        - {total}\n")
            file.write(f"        - :ref:`{link}`\n")

    @staticmethod
    def output_to_rst_footer(output_rst_file: str, tests: dict) -> None:
        with open(output_rst_file, "a") as file:
            file.write(
                "\nContents\n"
                "----------------------------------------------------------\n"
                ".. toctree::\n"
                "   :includehidden:\n"
                "   :maxdepth: 3\n"
                "\n")

            for key in tests:
                file.write(f"   {key}_report\n")


class TestCaseOverview:
    @staticmethod
    def output_rst_header(output_rst_file: str, file_name: str) -> None:
        with open(output_rst_file, "w") as file:
            file.write(
                f".. _{file_name}_report:\n"
                f"\n"
                f"Test Report for {file_name}\n"
                f"==========================================================\n"
                f"\n"
                f"\n"
                f".. admonition:: Test Report {file_name}\n\n"
                f"   .. list-table::\n"
                f"      :header-rows: 1\n"
                f"      :widths: 10 10 10 10 10\n\n"
                f"      * - UID\n"
                f"        - TESTED OS\n"
                f"        - COMMENTS\n"
                f"        - RESULT\n"
                f"        - TEST LINK\n")

    @staticmethod
    def output_to_rst_table(output_rst_file: str, uid: str, tested_os: str, comments: str, result: str,
                            link_to_file: str) -> None:
        with open(output_rst_file, "a") as file:
            file.write(f"      * - {uid}\n")
            file.write(f"        - {tested_os}\n")
            file.write(f"        - {comments}\n")
            file.write(f"        - {result}\n")
            file.write(f"        - {link_to_file}\n")


def maneuver(test_folder: str, test_report_dir: str, scopy_version: str) -> None:
    tests = {}
    test_folder_full = os.path.abspath(test_folder)
    test_report_dir_full = os.path.abspath(
        os.path.join(os.path.dirname(__file__), test_report_dir))

    if not os.path.exists(test_report_dir_full):
        os.mkdir(test_report_dir_full)

    for root, _, files in os.walk(test_folder_full):
        for file in files:
            if file.endswith('.rst'):
                with open(os.path.join(root, file), 'r') as f:
                    print(f"Processing {root}/{file}")
                    content = f.read()
                    # This mega regex is used to extract the test cases from
                    # the rst files.
                    matches = re.findall(
                        r'\*\*UID:?\*\*:?\s*(.*?)\n.*?OS:\s*(.*?)\n.*?\*\*Steps:?\*\*:? *\n(.*?)\*\*Tested OS:?\*\*:? ?(.*?)\n.*?\*\*Comments:?\*\*:? ?(.*?)\n.*?\*\*Result:?\*\*:? ?(.*?)\n',
                        content,
                        re.DOTALL)

                    pass_count = 0
                    fail_count = 0
                    skip_count = 0

                    if len(matches) == 0:
                        continue

                    file_name = file.removesuffix(".rst")
                    full_output_name = os.path.join(
                        test_report_dir_full, f"{file_name}_report.rst")
                    TestCaseOverview.output_rst_header(
                        output_rst_file=full_output_name,
                        file_name=file_name
                    )

                    for match in matches:
                        uid, _, _, tested_os, comments, result = match
                        TestCaseOverview.output_to_rst_table(
                            output_rst_file=full_output_name,
                            uid=uid,
                            tested_os=tested_os,
                            comments=comments,
                            result=result,
                            link_to_file=scopy_test_resources_base_dir(
                                scopy_version) + test_folder + os.path.relpath(root, test_folder_full) + "/" + file
                        )

                        if result == "PASS":
                            pass_count += 1
                        elif result == "FAIL":
                            fail_count += 1
                        else:
                            skip_count += 1

                    tests[file_name] = {
                        "pass": pass_count,
                        "fail": fail_count,
                        "skip": skip_count,
                        "total": len(matches)
                    }

    overview_file_name = os.path.join(
        test_report_dir_full, "test_report_overview.rst")
    TestSuiteOverview.output_rst_header(overview_file_name)

    for key in tests:
        TestSuiteOverview.output_to_rst_table(
            output_rst_file=overview_file_name,
            name=key,
            link=key + "_report",
            passed=tests[key]["pass"],
            failed=tests[key]["fail"],
            skipped=tests[key]["skip"],
            total=tests[key]["total"]
        )

    # Create the index for all test report files
    index_file_name = os.path.join(test_report_dir_full, "index.rst")
    with open(index_file_name, "w") as file:
        file.write(".. _test_report:\n\n")
        file.write("Test Report\n")
        file.write(
            "========================================================\n\n")
        file.write(".. include:: test_report_overview.rst\n\n")
        file.write("Contents\n")
        file.write(
            "----------------------------------------------------------\n")
        file.write(".. toctree::\n")
        file.write("   :maxdepth: 3\n")
        file.write("\n")
        for key in tests:
            file.write(f"   {key}_report\n")

    # Reference the created index in the above level
    link_index = os.path.join(test_report_dir_full, "../index.rst")
    with open(link_index, "a") as file:
        file.write("   test_report/index\n")


def main() -> None:
    if len(sys.argv) != 4:
        print("Usage: python test_report_generator.py <docs_root_folder> <testing_results_dir> <scopy_version>")
        sys.exit(1)

    test_folder = sys.argv[1]
    test_output_dir = sys.argv[2]
    scopy_version = sys.argv[3]

    maneuver(test_folder=test_folder, test_report_dir=test_output_dir,
             scopy_version=scopy_version)


if __name__ == "__main__":
    main()
