# /create-whatsnew — Generate What's New HTML page

You are creating a "What's New" HTML page for a Scopy plugin release.

**Arguments:** `$ARGUMENTS`
(Format: `<plugin> <version>` — e.g., `ad9371 V2.2.0`)

## Step 0: Parse arguments and load context

Parse `$ARGUMENTS` to extract:
- **Plugin name** — first word (e.g., `ad9371`)
- **Version** — second word (e.g., `V2.2.0`)

Use the Read tool to check if a port state file exists:
- Path: `tasks/<plugin>-port-state.md` (substitute the parsed plugin name)
- If the file does not exist, note "No state file — will discover from plugin docs." and continue.

Use the Glob tool to check if the version folder exists:
- Pattern: `scopy/resources/whatsnew/<version>/` (substitute the parsed version)
- If nothing is found, note "Version folder does not exist yet — will create it."

## Step 1: Discovery

**Feature inventory** (in priority order):

1. **State file** (if exists) — sections list, attribute inventory, device name, all features
2. **Plugin docs** (if exist) — `scopy/docs/plugins/<plugin>/index.rst` and `<plugin>.rst` — feature list, device support, key parameters
3. **manifest.json.cmakein** — `display_name`, `description`, `device`

Read the reference prompt for HTML patterns and validation:
`scopy/tools/prompts/scopy_whatsnew_agent.md`

Study these reference HTML files before writing:
- `scopy/resources/whatsnew/V2.2.0/adrv9002_plugin.html` — comprehensive example (RF transceiver)
- `scopy/resources/whatsnew/V2.2.0/imuanalyzer_plugin.html` — simpler example
- `scopy/resources/whatsnew/V2.2.0/rfpowermeter.html` — another style

Study the QRC file for how to add entries:
- `scopy/resources/whatsnew/whatsnew.qrc`

## Step 2: Draft — WAIT FOR APPROVAL

Present the HTML content draft:

1. **Title** — `<h1><Device Display Name> Plugin</h1>`
2. **Overview paragraph** — what the plugin does, what device it supports
3. **Key Features list** — `<h2>Key Features</h2>` with `<ul>/<li>` — one item per major section
4. **Optional extra section** — only if plugin has a notable subsystem
5. **How to Use** — `<h2>How to Use</h2>` with numbered `<ol>/<li>` steps
6. **Documentation link** — `<a href="https://analogdevicesinc.github.io/scopy/plugins/<plugin>/<plugin>.html">`
7. **QRC entry** to add: `<file>VERSION/<plugin>_plugin.html</file>`

**Wait for user approval before writing any files.**

## Step 3: Implement

### 3.1 Create directory (if needed)
If `scopy/resources/whatsnew/<version>/` does not exist, create it.

### 3.2 Write the HTML file
Create `scopy/resources/whatsnew/<version>/<plugin>_plugin.html`

**HTML template:**
```html
<h1><Device Display Name> Plugin</h1>

<p>Scopy now offers dedicated support for the <Device Name>.</p>

<p>The new <b><Tool Display Name> Tool</b> enables you to integrate and control <Device>-based devices within Scopy. <One sentence describing the key capability.></p>

<h2>Key Features</h2>
<ul>
  <li><b>Section Name:</b> Brief description of what this section controls</li>
</ul>

<h2>How to Use</h2>
<ol>
  <li>Connect your <Device> device to your computer.</li>
  <li>Ensure the <Device> IIO drivers are properly installed.</li>
  <li>Open Scopy and select the <b><Tool Name></b> tool from the available instruments.</li>
</ol>

<p>For more detailed information, please visit the <a href="https://analogdevicesinc.github.io/scopy/plugins/<plugin>/<plugin>.html" target="_blank">official documentation</a>.</p>
```

**Rules for HTML content:**
- Use `<b>` for bold emphasis — no CSS, no inline styles
- Use `<h1>` for the title, `<h2>` for sections — no `<h3>` or deeper
- Use `<ul>/<li>` for feature lists, `<ol>/<li>` for how-to steps
- No `<!DOCTYPE>`, `<html>`, `<head>`, or `<body>` tags — this is an HTML fragment
- Keep Key Features list to 4-8 items

### 3.3 Update the QRC file
Edit `scopy/resources/whatsnew/whatsnew.qrc` to add the new `<file>` entry in alphabetical order within the version group.

## Step 4: Validate

- [ ] HTML is a fragment (no `<html>/<head>/<body>` tags)
- [ ] Only `<h1>` and `<h2>` heading levels used
- [ ] Documentation link URL follows the exact pattern (no typos in plugin name)
- [ ] `<file>` entry added to `whatsnew.qrc` in the correct version group
- [ ] QRC entries within the version block are in alphabetical order
- [ ] HTML renders correctly (balanced tags, no unclosed elements)
- [ ] Plugin display name matches `manifest.json.cmakein` `display_name` field

## Step 5: Update state file (if it exists)

```markdown
## Status
- Phase: WHATSNEW_COMPLETE
```

## Rules

- Do NOT add CSS styles or JavaScript to the HTML
- Do NOT use image tags — no screenshots or diagrams
- Do NOT use `<h3>` or deeper heading levels
- Keep the "How to Use" section generic (connect -> install drivers -> open Scopy -> select tool)
- The documentation link must point to `analogdevicesinc.github.io/scopy` — do not guess other URLs
- Keep Key Features concise — each bullet should be one sentence maximum
