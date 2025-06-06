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

def setup(app):
    app.add_directive("hello-world", HelloWorldDirective)
    app.add_directive("test-results-folder", TestResultsFolderDirective)