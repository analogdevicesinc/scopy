Create automated JavaScript test scripts for the Scopy plugin: $ARGUMENTS

Follow the instructions in @tools/prompts/scopy_test_automation_agent.md exactly.
Also use @tools/prompts/scopy_test_automation_prompt.md as a reference for templates and patterns.

## Workflow

1. **Discover**: Find the plugin's API header (`*_api.h`) and its test documentation RST file (`*_tests.rst`). Read both completely. Also read the JS API object name from `js/testAutomations/common/apiUnregisterTest.js` or from the plugin source code.

2. **Classify**: For every test in the RST documentation, classify it as:
   - **Category A** (Fully Automatable) — steps AND validation via API
   - **Category B** (Supervised/Visual) — steps via API, validation requires human observer
   - **Category C** (Not Automatable) — missing API calls or requires external tools
   Present the full classification report to me for review BEFORE writing any code.

3. **Implement**: After I approve the classification, create:
   - `<plugin>DocTests.js` for Category A tests
   - `<plugin>VisualTests.js` for Category B tests (using `TestFramework.supervisedCheck()` for user y/n input)
   - A "Missing API Report" comment block for Category C tests at the top of DocTests.js

4. **Validate**: Run through the checklist from the agent prompt to verify completeness.

## Rules
- Do NOT modify any C++ source code — only create JS test files under `js/testAutomations/`.
- Do NOT invent API methods that do not exist — check the `*_api.h` header for available `Q_INVOKABLE` methods.
- ALWAYS save and restore original values in every test.
- ALWAYS use `msleep(500)` after every setter call.
- Use `TestFramework.supervisedCheck()` for visual validation instead of `msleep(VISUAL_DELAY)`.
- Use the existing test file patterns in `js/testAutomations/` as references.
- Use doc UIDs as test names in `TestFramework.runTest()`.
