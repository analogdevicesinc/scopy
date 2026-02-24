Create a JavaScript API class for the Scopy plugin: $ARGUMENTS

Follow the instructions in @prompts/scopy_api_agent.md exactly.

## Workflow

1. **Analyze**: Read the plugin's header, source, and all instrument class headers to understand its structure. Identify tool IDs, instrument classes, and all private members that can be exposed.

2. **Design**: Build a complete method list and present it to me for review BEFORE writing any code. Also present a "Missing Functionality Report" listing anything that cannot be exposed without modifying base classes.

3. **Implement**: After I approve the design, create the API header, API source, and the minimal modifications to the plugin header/source (friend class + initApi only).

4. **Validate**: Run through the checklist from the agent prompt to verify completeness.

## Rules
- Do NOT modify any instrument or utility class — the API is a new layer on top.
- Do NOT create methods that require new functions in base classes — flag them in the missing functionality report instead.
- Always null-check instrument pointers before accessing them.
- Use the existing code patterns from the reference implementations.
