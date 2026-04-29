# /clarify-task — Refine a vague Scopy development task into a structured task brief

> **Recommended model:** Opus — this command performs deep codebase exploration and multi-step reasoning to produce high-quality task briefs.

You are helping the user turn a vague or incomplete development idea into a concrete, actionable task brief for the Scopy project.

**Raw task description:** `$ARGUMENTS`

## Step 0: Load context

Load these knowledge skills by reading their SKILL.md files:
- `scopy-architecture-knowledge` — core architecture patterns
- `scopy-component-relationships` — how components interact
- `scopy-decision-frameworks` — architectural decision trees
- `external-research` — guidelines for researching external solutions

## Step 1: Initial analysis

Classify the task into one or more categories:
- **New plugin/package** — creating something from scratch
- **Modification to existing plugin** — changing existing functionality
- **Core infrastructure change** — modifying shared libraries
- **Tooling/CI change** — build system, scripts, CI pipelines
- **Documentation** — docs, tests, guides
- **Cross-cutting concern** — affects multiple modules

Based on classification, identify which Scopy modules/packages are likely involved:
- Use the Glob tool to find relevant source files and headers
- Use the Grep tool to search for related patterns, class names, or keywords
- Read key files (plugin headers, tool classes, CMakeLists.txt) to understand current state

If the task mentions external technologies, unfamiliar concepts, or problems that might have existing solutions outside Scopy:
- Use web search or Context7 to gather context
- Check `https://analogdevicesinc.github.io` for ADI hardware/driver documentation
- Note findings for inclusion in the task brief

## Step 2: Clarifying questions

Ask **3-6 clarifying questions**, one at a time, using the AskUserQuestion tool. Each question should be informed by what you learned from code exploration.

**Question categories (use as needed, not a fixed script):**

1. **Scope** — "What exactly should change? What should NOT change?"
   - Informed by: what you found in the code (e.g., "I see the ADC plugin has 3 tools — are we adding to an existing tool or creating a new one?")

2. **Users** — "Who uses this feature?"
   - Options: end user (Scopy GUI), developer (code/API), tester (automation), CI system

3. **Constraints** — "Are there hardware requirements, platform considerations, or performance needs?"
   - Informed by: device compatibility, IIO attributes available, cross-platform concerns

4. **Integration** — "How does this interact with existing plugins/tools?"
   - Informed by: MessageBroker topics, shared widgets, library dependencies

5. **Success criteria** — "How do we know it's done? What does 'working' look like?"
   - Help the user define concrete acceptance criteria

6. **Priority/timeline** — "Is this blocking something? What's the urgency?"

**Guidelines:**
- Prefer multiple-choice questions (use AskUserQuestion options) when possible
- One question per message — do not combine questions
- Adapt based on previous answers — skip irrelevant categories
- Reference specific code/files you found during exploration to make questions concrete

## Step 3: Deep exploration

After all questions are answered, do a final targeted code exploration:
- Read relevant plugin/library source files identified during Q&A
- Identify existing patterns, utilities, or widgets that can be reused
- Find similar implementations in other plugins (cross-reference)
- Search externally if the task requires technology or patterns not in the codebase
- Note any potential conflicts or dependencies

## Step 4: Generate task brief

Present the following structured task brief in the conversation:

```markdown
## Task Brief: <descriptive title>

**Type:** <new feature | modification | infrastructure | tooling | documentation | cross-cutting>
**Modules affected:** <list of packages/libraries>
**Summary:** <2-3 sentence refined description of what needs to be done>

### Requirements
- <concrete, testable requirement 1>
- <concrete, testable requirement 2>
- ...

### Existing Resources
- <file/pattern/utility> — <why it's useful for this task>
- ...

### External Resources (if applicable)
- <library/doc/pattern found via research> — <relevance>
- ...

### Key Decisions Made
- <decision from Q&A with rationale>
- ...

### Open Questions (if any)
- <things that still need resolution before design>

### Suggested Next Step
> Run `/scopy_dev_plugin:design-task` with this brief to generate a full design document with architectural choices, component design, and testing strategy.
```

## Step 5: Save (optional)

Ask the user if they want to save the task brief:

> "Would you like me to save this task brief to `docs/design/YYYY-MM-DD-<topic>-brief.md`?"

If yes, write the file. Do NOT commit — let the user decide when to commit.

## Rules

- Do NOT write any code or modify source files — this command produces only a task brief
- Do NOT skip the clarifying questions phase — even if the task seems clear, validate with the user
- Do NOT ask more than 6 questions — consolidate if needed
- Always explore the codebase before asking questions — informed questions are better questions
- Always check for external solutions when the task involves unfamiliar technology
- Reference specific files and line numbers when discussing existing code
