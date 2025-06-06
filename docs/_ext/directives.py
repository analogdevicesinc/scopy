from docutils.parsers.rst import Directive
from docutils import nodes
import os
import test_report_generator

class HelloWorldDirective(Directive):
    def run(self):

        os.environ['TEST_RESULTS_GENERATION'] = 'true'

        if os.environ.get('TEST_RESULTS_GENERATION', 'false').lower() == 'true':
            return [nodes.paragraph(text='Hello, Sphinx Directive!')]
    
        return [nodes.paragraph(text='no build done')]

class TestResultsFolderDirective(Directive):
    def run(self):
        # test purposes
        os.environ['TEST_RESULTS_GENERATION'] = 'false'
        
        # Only generate the table if TEST_RESULTS_GENERATION is set to 'true'
        if os.environ.get('TEST_RESULTS_GENERATION', 'false').lower() != 'true':
            return [nodes.paragraph(text="Test results generation is disabled for this build.")]

        # # Replicate: TEST_REPORT_FOLDER=$(echo "${{ github.event.pull_request.head.ref }}" | sed 's/^testing_//')
        # pr_head_ref = os.environ.get('GITHUB_PR_HEAD_REF', '')
        # test_report_folder = pr_head_ref.removeprefix('testing_') if pr_head_ref.startswith('testing_') else pr_head_ref

        # # Log for debugging
        # print(f"test_report_folder={test_report_folder}")

        # # Prepare arguments for test_report_generator
        # base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
        # docs_root = os.path.abspath(os.path.join(base_dir, '..', 'testing_results', test_report_folder))
        # output_dir = os.path.join(base_dir, 'tests', 'test_report')
        # scopy_version = os.environ.get('GITHUB_SHA', 'local')

        # try:
        #     test_report_generator.maneuver(docs_root, output_dir, scopy_version)
        # except Exception as e:
        #     return [nodes.paragraph(text=f"Test report generation failed: {e}")]
        # return [nodes.paragraph(text=f"Test report generated in tests/test_report for folder: {test_report_folder}")]

class GenerateTestReportDirective(Directive):
    def run(self):
        import io
        test_folder = os.environ.get('GENERATE_TEST_REPORT_TEST_FOLDER', '../../testing_results')
        test_output_dir = os.environ.get('GENERATE_TEST_REPORT_OUTPUT_DIR', 'tests/test_report')
        scopy_version = os.environ.get('GENERATE_TEST_REPORT_VERSION', 'local')
        try:
            rst_output = io.StringIO()
            def fake_output_rst_header(output_rst_file, file_name=None):
                if file_name is not None:
                    # TestCaseOverview
                    rst_output.write(
                        f".. _{file_name}_report:\n\n"
                        f"Test Report for {file_name}\n"
                        f"{'='*58}\n\n"
                        f".. admonition:: Test Report {file_name}\n\n"
                        f"   .. list-table::\n"
                        f"      :header-rows: 1\n"
                        f"      :widths: 10 10 10 10 10\n\n"
                        f"      * - UID\n"
                        f"        - TESTED OS\n"
                        f"        - COMMENTS\n"
                        f"        - RESULT\n"
                        f"        - TEST LINK\n"
                    )
                else:
                    # TestSuiteOverview
                    rst_output.write(
                        f".. admonition:: Test Report {output_rst_file}\n\n"
                        f"   .. list-table::\n"
                        f"      :header-rows: 1\n"
                        f"      :widths: 50 30 30 50 50 50\n\n"
                        f"      * - NAME\n"
                        f"        - PASS\n"
                        f"        - FAIL\n"
                        f"        - SKIP\n"
                        f"        - Total\n"
                        f"        - Details\n"
                    )
            def fake_output_to_rst_table(output_rst_file, *args, **kwargs):
                if len(args) == 6:
                    name, link, passed, failed, skipped, total = args
                    rst_output.write(
                        f"      * - {name}\n"
                        f"        - {passed}\n"
                        f"        - {failed}\n"
                        f"        - {skipped}\n"
                        f"        - {total}\n"
                        f"        - :ref:`{link}`\n"
                    )
                elif len(args) == 5:
                    uid, tested_os, comments, result, link_to_file = args
                    rst_output.write(
                        f"      * - {uid}\n"
                        f"        - {tested_os}\n"
                        f"        - {comments}\n"
                        f"        - {result}\n"
                        f"        - {link_to_file}\n"
                    )
            def fake_output_to_rst_footer(output_rst_file, tests):
                rst_output.write(
                    "\nContents\n"
                    "----------------------------------------------------------\n"
                    ".. toctree::\n"
                    "   :includehidden:\n"
                    "   :maxdepth: 3\n"
                    "\n"
                )
                for key in tests:
                    rst_output.write(f"   {key}_report\n")
            # Patch the TestSuiteOverview and TestCaseOverview methods
            orig_suite_header = test_report_generator.TestSuiteOverview.output_rst_header
            orig_suite_table = test_report_generator.TestSuiteOverview.output_to_rst_table
            orig_suite_footer = test_report_generator.TestSuiteOverview.output_to_rst_footer
            orig_case_header = test_report_generator.TestCaseOverview.output_rst_header
            orig_case_table = test_report_generator.TestCaseOverview.output_to_rst_table
            test_report_generator.TestSuiteOverview.output_rst_header = staticmethod(lambda output_rst_file: fake_output_rst_header(output_rst_file))
            test_report_generator.TestSuiteOverview.output_to_rst_table = staticmethod(fake_output_to_rst_table)
            test_report_generator.TestSuiteOverview.output_to_rst_footer = staticmethod(fake_output_to_rst_footer)
            test_report_generator.TestCaseOverview.output_rst_header = staticmethod(lambda output_rst_file, file_name: fake_output_rst_header(output_rst_file, file_name))
            test_report_generator.TestCaseOverview.output_to_rst_table = staticmethod(fake_output_to_rst_table)
            # Run the maneuver function, but patch out file writing in test_report_generator
            orig_open = open
            def fake_open(*args, **kwargs):
                class DummyFile:
                    def write(self, *_): pass
                    def __enter__(self): return self
                    def __exit__(self, *a): pass
                return DummyFile()
            import builtins
            builtins_open = builtins.open
            builtins.open = fake_open
            try:
                test_report_generator.maneuver(test_folder, test_output_dir, scopy_version)
            finally:
                builtins.open = builtins_open
            # Restore the original methods
            test_report_generator.TestSuiteOverview.output_rst_header = orig_suite_header
            test_report_generator.TestSuiteOverview.output_to_rst_table = orig_suite_table
            test_report_generator.TestSuiteOverview.output_to_rst_footer = orig_suite_footer
            test_report_generator.TestCaseOverview.output_rst_header = orig_case_header
            test_report_generator.TestCaseOverview.output_to_rst_table = orig_case_table
            # Return the full RST as a literal block
            rst_content = rst_output.getvalue()
            return [nodes.literal_block(rst_content, rst_content)]
        except Exception as e:
            return [nodes.paragraph(text=f"Test report generation failed: {e}")]

def setup(app):
    app.add_directive("hello-world", HelloWorldDirective)
    app.add_directive("test-results-folder", TestResultsFolderDirective)
    app.add_directive("generate-test-report", GenerateTestReportDirective)