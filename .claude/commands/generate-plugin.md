Generate a complete Scopy plugin package from an iio_info text file: $ARGUMENTS

Follow the instructions in @/prompts/plugin_generator_agent.md exactly.

## Workflow

1. **Parse**: Read the iio_info file, extract all devices, channels, and attributes. Present a structured summary (device names, channel counts, attribute counts) for my review.

2. **Scope**: Propose the plugin name, namespace, tool organization (which devices map to which tools), and file structure. Ask me to approve BEFORE proceeding.

3. **Design**: Present the complete attribute-to-widget mapping for every attribute: which IIOWidgetBuilder strategy to use, which attributes to skip, which need unit conversions. Ask me to approve BEFORE writing any code.

4. **Generate**: Create all files in the correct directory structure under `packages/`. This includes: package CMakeLists.txt, manifest.json.cmakein, plugin CMakeLists.txt, config header, export header setup, plugin class, tool class, helper class, API class, test files, resources, and EMU-XML.

5. **Validate**: Run through the completeness checklist from the agent prompt. Report any issues found.

## Rules
- Do NOT invent attributes that are not present in the iio_info file.
- Do NOT skip the interactive approval steps — always present the mapping and wait for confirmation.
- Follow the ad936x plugin as the canonical reference for all patterns and conventions.
- Every IIOWidgetBuilder chain MUST include `.group(m_mgr)` before `.buildSingle()`.
- Every IIOWidget MUST be connected to the `readRequested` signal.
- Use the existing Scopy code style: tabs for indentation, m_ prefix for members, Scopy copyright header on all files.
