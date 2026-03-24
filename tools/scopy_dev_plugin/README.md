# scopy_dev_plugin

Claude Code plugin for Scopy development. Provides slash commands, knowledge skills, and hooks for building IIO device plugins.

The plugin lives inside the repository at `tools/scopy_dev_plugin/`. It is distributed via the local marketplace manifest at `.claude-plugin/marketplace.json`.

## Setup (per developer)

Run these three commands inside Claude Code from the repo root:

```
/plugin marketplace add ./
/plugin install scopy_dev_plugin@scopy-plugins --scope local
/reload-plugins
```

1. `/plugin marketplace add ./` — registers the local marketplace (reads `.claude-plugin/marketplace.json`)
2. `/plugin install scopy_dev_plugin@scopy-plugins --scope local` — installs the plugin into your local settings
3.  Restart Claude Code

> `settings.local.json` is gitignored — each developer runs the install once. Do not commit it.

## Requirements

- `clang-format` — required by the formatting hook (C++/header files)
- `cmake-format` — required by the formatting hook (CMake files)

Both must be on your `PATH`. On Ubuntu: `sudo apt install clang-format` and `pip install cmake-format`.

## Slash Commands

| Command | Description |
|---------|-------------|
| `/scopy_dev_plugin:create-api <plugin>` | Generate JavaScript API class for test automation |
| `/scopy_dev_plugin:create-docs <plugin>` | Generate RST plugin user-guide documentation |
| `/scopy_dev_plugin:create-test-docs <plugin>` | Generate RST test case documentation |
| `/scopy_dev_plugin:create-automated-tests <plugin>` | Create JS automated test scripts |
| `/scopy_dev_plugin:create-whatsnew <plugin> <version>` | Generate What's New HTML page |
| `/scopy_dev_plugin:check-code-quality <package>` | Static code quality analysis |
| `/scopy_dev_plugin:verify-package <package>` | CI pre-flight validation (format + license) |
| `/scopy_dev_plugin:validate-api <plugin>` | Validate API class implementation (checks A1–A7) |
| `/scopy_dev_plugin:validate-automated-tests <plugin>` | Validate JS automated test scripts (checks T1–T7) |

## Knowledge Skills (auto-load)

These skills are loaded automatically when relevant context is detected:

- **iiowidget-patterns** — How to create IIOWidgets using IIOWidgetBuilder
- **scopy-style-rules** — Theme system, Style::setStyle, background colors
- **scopy-plugin-patterns** — Plugin lifecycle, ToolTemplate, refresh buttons
- **scopy-doc-format** — RST documentation conventions
- **scopy-test-format** — Test case UID and RBP conventions
- **scopy-api-patterns** — API class structure and Q_INVOKABLE patterns

## Hooks

Hooks run automatically after file edits — no manual invocation needed.

- **scopy-format.sh** — Formats C++ and CMake files after any Edit/Write/MultiEdit
- **scopy-license.sh** — Adds GPL license header to newly created source files

### Hook deduplication

If you have the same hooks configured globally in `~/.claude/settings.local.json`, they will fire twice. Remove the duplicates from your global settings to avoid this.
