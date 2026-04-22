# /design-task — Generate an architectural design document for a Scopy development task

> **Recommended model:** Opus — this command performs complex architectural reasoning and produces detailed design documents.

You are creating a detailed design document for a Scopy development task. The input may be a refined task brief (from `/clarify-task`) or a direct task description.

**Task description:** `$ARGUMENTS`

## Step 0: Load context

Load ALL knowledge skills by reading their SKILL.md files:
- `scopy-architecture-knowledge` — core architecture patterns
- `scopy-tools-inventory` — available development tools
- `scopy-component-relationships` — how components interact
- `scopy-decision-frameworks` — architectural decision trees
- `external-research` — guidelines for researching external solutions

Check if a task brief exists in `docs/design/` matching the topic:
- Use Glob to search for `docs/design/*brief.md` files
- If found, read it and use it as the primary input

If `$ARGUMENTS` appears vague or underspecified (no concrete requirements, unclear scope):
- Suggest running `/scopy_dev_plugin:clarify-task` first
- But do NOT block — proceed if the user wants to continue

## Step 1: Architecture analysis

Using the decision frameworks skill, work through key architectural decisions:

1. **Where does this belong?**
   - Which package, plugin, or core library?
   - Use the "Plugin vs Core Library" and "New Package vs Extend Existing" decision trees

2. **What components need to be created vs modified?**
   - Use Glob/Grep to find existing files that will be affected
   - Read relevant source files to understand current structure

3. **What patterns from the codebase apply?**
   - ToolTemplate, IIOWidgetBuilder, MessageBroker, Preferences, etc.
   - Use the "Tool vs Section", "IIOWidget vs Custom Widget", "Sync vs Async" decision trees

4. **What external dependencies are needed?**
   - If any, follow external-research skill guidelines
   - Check license compatibility, cross-platform support

**Present 2-3 architectural approaches** with trade-offs and your recommendation. Use AskUserQuestion to let the user choose. Format:

```markdown
### Approach A: <name> (Recommended)
- **How it works:** <brief description>
- **Pros:** <list>
- **Cons:** <list>

### Approach B: <name>
- **How it works:** <brief description>
- **Pros:** <list>
- **Cons:** <list>
```

**Wait for user to choose before continuing.**

## Step 2: Component design

For the chosen approach, detail each component:

### New Classes
For each new class:
- **Name** (following Scopy conventions: PascalCase, m_ prefix for members)
- **Responsibility** (single responsibility)
- **Inheritance** (PluginBase, ApiObject, QWidget, etc.)
- **Key methods** (with signatures)
- **File location** (following lowercase filename convention)

### Modified Classes
For each modified class:
- **File path** and current structure
- **What changes** and why
- **Impact** on other consumers of this class

### File Locations
Map all new and modified files:
```
packages/<name>/plugins/<plugin>/
├── include/<plugin>/
│   ├── newclass.h          (NEW)
│   └── existingclass.h     (MODIFIED)
├── src/
│   ├── newclass.cpp         (NEW)
│   └── existingclass.cpp    (MODIFIED)
├── CMakeLists.txt           (MODIFIED if explicit file lists)
└── test/
    └── tst_newfeature.cpp   (NEW)
```

### Data Flow
Describe how components communicate:
- Qt signals/slots connections
- MessageBroker topics (if cross-plugin)
- IIO data flow (attribute read/write, buffer streaming)
- API exposure for test automation

### Configuration
- CMake changes needed (new targets, dependencies, flags)
- manifest.json changes (if any)
- Preferences entries (if any)

## Step 3: Existing tools & patterns report

Using the tools inventory skill, categorize what's available:

### Reusable (use directly)
- Existing utilities, widgets, or patterns that solve part of the problem
- Reference specific files and explain how to use them

### Adaptable (modify slightly)
- Things that exist but need small changes
- Explain what changes are needed and why

### Missing (build from scratch)
- What doesn't exist and must be created
- Estimate relative complexity (simple/moderate/complex)

### External (from outside Scopy)
- Libraries or patterns found via research
- Use the format from the external-research skill
- Include "build it ourselves" as an alternative

## Step 4: Testing & documentation strategy

### Unit Tests
- What to test with Qt Test framework
- Mock strategy (if applicable)
- File locations in `test/` subdirectory

### JS Hardware-in-the-Loop Tests
- What to test with JS HITL (patterns from `claude_scopy_dev_plugin` branch)
- API methods needed for automation
- Hardware requirements

### Manual Test Cases
- RST test documentation with UIDs and RBP priority levels
- Category A (automatable), B (visual), C (manual only) breakdown

### API Class
- Is a Q_INVOKABLE API class needed?
- What methods to expose
- Conversion rules for IIO attributes

### Documentation
- User guide sections (RST format)
- What's New entry for releases

## Step 5: Present design

Present the full design in conversation, organized as:

1. **Architecture Overview** — chosen approach, component diagram
2. **Component Details** — new/modified classes, file locations
3. **Data Flow** — how components communicate
4. **Existing Tools Report** — reusable/adaptable/missing/external
5. **Testing Strategy** — unit, HITL, manual, API
6. **Implementation Order** — suggested sequence of implementation steps

Ask after each major section: "Does this look right so far?" using AskUserQuestion.

## Step 6: Save design document

Ask the user:
> "Would you like me to save this design document to `docs/design/YYYY-MM-DD-<topic>-design.md`?"

If yes, write the complete design document to file.

## Step 7: Transition

After the design is complete and saved, suggest next steps:

> **Next steps:**
> - Run the `writing-plans` skill to create a detailed implementation plan from this design
> - Or use specific commands for individual pieces:
>   - `/scopy_dev_plugin:create-api <plugin>` for the API class
>   - `/scopy_dev_plugin:create-docs <plugin>` for documentation
>   - `/scopy_dev_plugin:create-automated-tests <plugin>` for test automation

## Rules

- Do NOT write any code or modify source files — this command produces only a design document
- Always present 2-3 approaches before detailing one — let the user choose
- Always check for existing patterns before proposing new ones
- Always evaluate external solutions using the external-research skill criteria
- Reference specific files and line numbers when discussing existing code
- Follow Scopy naming conventions in all proposed class/file names
- Include testing strategy — every feature needs a test plan
- The design document should be detailed enough that someone could implement from it
