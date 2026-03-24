# /create-docs — Generate RST plugin user-guide documentation

You are creating Sphinx RST documentation for a Scopy plugin user guide.

**Plugin:** `$ARGUMENTS`

## Step 0: Load context

Use the Read tool to check if a port state file exists:
- Path: `tasks/$ARGUMENTS-port-state.md`
- If the file does not exist, note "No state file — will discover from source files directly." and continue.

## Step 1: Discovery

**If state file exists**, extract from it:
- Plugin display name, device name, and JS API object name
- Section list (all tabs and sub-tabs)
- Attribute inventory (all attribute names, widget types, IIO paths)
- Whether an advanced tab is present

**If no state file**, discover directly from source files:
- Use the Read tool on `manifest.json.cmakein` for `display_name`, `description`, `device` fields
- Read tool class headers for section names (look for `QLabel("Section Name"...)` or `MenuSectionCollapseWidget(...)`)
- Read widget factory (if present) for attribute names and descriptions
- Read plugin header for namespace and class name

Study these reference RST files before writing anything:
- `scopy/docs/plugins/ad936x/index.rst` — index structure
- `scopy/docs/plugins/ad936x/ad936x.rst` — main doc structure and attribute list format
- `scopy/docs/plugins/ad936x/ad936x_advanced.rst` — advanced tab doc format (if applicable)
- `scopy/docs/plugins/index.rst` — top-level toctree to update

## Step 2: Plan — WAIT FOR APPROVAL

Present a documentation outline:

1. **File list** to create:
   - `scopy/docs/plugins/$ARGUMENTS/index.rst`
   - `scopy/docs/plugins/$ARGUMENTS/$ARGUMENTS.rst`
   - `scopy/docs/plugins/$ARGUMENTS/$ARGUMENTS_advanced.rst` (only if advanced tab exists)
2. **Sections** to document (one subsection per plugin section)
3. **Attributes per section** with brief descriptions (no wiki links)
4. **Screenshot list** — for every section and key UI element, list the screenshot the user should take
5. **Cross-reference label** (`.. _<plugin>:`) and RST label naming convention
6. **toctree update** in `scopy/docs/plugins/index.rst` (show exact line to add)
7. **Supported devices** list (from manifest or state file)

**Wait for user approval before writing any RST.**

## Step 3: Implement

Follow the RST conventions from the reference files:
- Use `================================================================================` (80 `=`) for top-level headings
- Use `---------------------------------` for section headings
- Attribute entries use bullet list with **bold name** and colon: `- **Attribute Name:** Description`
- Do NOT include wiki links — descriptions are self-contained
- `.. _<plugin>:` ref label at top of main RST file
- Index file must have overview, features, supported devices, getting started, toctree

### Image placeholders

Every section and major feature must have an image placeholder:

```rst
.. AN_IMAGE_WILL_BE_HERE
   Screenshot: <exact description of what to capture>
```

Place image placeholders:
- Once at the top of the main RST file (full plugin overview screenshot)
- Once per major section (showing that section's controls)
- Once for the advanced tab (if it exists)

### index.rst structure
```rst
.. _<plugin>_index:

<Device Name>
========================================

Overview
--------
<One paragraph about the plugin.>

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The <Plugin Name> tool open in Scopy showing the full interface with a connected device

Features
--------
- **Feature 1:** Description
- **Feature 2:** Description

Supported Devices
-------------------
- **<Device Family>** (list variants)

Getting Started
---------------
**Prerequisites**
- <hardware requirement>
- Scopy version X.Y or later

**Using the Plugin**
<brief how-to paragraph>

.. toctree::
   :maxdepth: 2

   <plugin>
   <plugin>_advanced      <- only if advanced tab exists
```

### After generating each RST file

Print a numbered screenshot checklist to the user.

### Toctree update in scopy/docs/plugins/index.rst
Add the plugin to the relevant group in both the bullet list at top and the toctree block at the bottom.

## Step 4: Validate

- [ ] All RST cross-reference labels are unique (not already used in the codebase)
- [ ] toctree entries match actual file names (no `.rst` extension in toctree)
- [ ] `index.rst` toctree includes all created files
- [ ] `scopy/docs/plugins/index.rst` has the new plugin in the bullet list and toctree
- [ ] Every attribute from the state file (or source discovery) appears in the docs
- [ ] Heading underlines are the correct length
- [ ] Every section has an `AN_IMAGE_WILL_BE_HERE` placeholder with a descriptive caption
- [ ] No wiki links present anywhere in the generated RST
- [ ] Screenshot checklist printed to user after file creation

## Step 5: Update state file (if it exists)

```markdown
## Status
- Phase: DOCS_COMPLETE
```

## Rules

- Match the exact style of `scopy/docs/plugins/ad936x/ad936x.rst`
- Do NOT use tabs — use 2-space indentation for RST lists
- Do NOT include wiki links — attribute descriptions must be self-contained
- Do NOT use `.. figure::` or `.. image::` directives — use `.. AN_IMAGE_WILL_BE_HERE` comments only
- Keep descriptions accurate to what the plugin actually does
- Advanced doc is only created if the plugin actually has an advanced tab
- Every section must have exactly one image placeholder
