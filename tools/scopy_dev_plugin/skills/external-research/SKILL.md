---
name: external-research
description: Guidelines for using Context7, web search, and other research tools to find solutions outside the Scopy codebase. Loaded when the task involves unfamiliar technology or external libraries.
---

# External Research Guidelines

## When to Search Externally

Search outside the Scopy codebase when:
- The task mentions technology not currently used in Scopy
- A third-party library could solve the problem better than custom code
- The task references protocols, standards, or algorithms you need documentation for
- You need hardware/driver documentation for a specific ADI device
- A Qt pattern or technique might exist that the codebase doesn't currently use
- Signal processing algorithms need reference implementations

## Where to Look

### ADI Hardware & Driver Documentation
**Primary source:** `https://analogdevicesinc.github.io`
- Device datasheets and user guides
- Linux IIO driver documentation
- Reference design documentation
- HDL/FPGA documentation

**Use when:** Task involves a specific ADI device, IIO driver attributes, hardware capabilities, or register maps.

### Qt Documentation
**Use Context7 or web search for:**
- Qt5 class reference (widgets, signals/slots, threading)
- Qt design patterns (Model/View, State Machine, Animation)
- QML integration patterns
- Cross-platform considerations

**Use when:** Building UI components, choosing between Qt approaches, or using Qt classes not currently in the codebase.

### libiio Documentation
**Use web search for:**
- libiio API reference
- IIO attribute types and conventions
- Context/device/channel hierarchy
- Buffer management patterns

**Use when:** Working with IIO device communication, attribute reads/writes, or buffer management.

### GNU Radio Documentation
**Use Context7 or web search for:**
- Block development patterns
- Flowgraph design
- Signal processing blocks reference
- Custom block creation

**Use when:** Task involves data streaming, signal processing pipelines, or extending gr-util.

### GitHub Reference Implementations
**Search GitHub for:**
- Similar Qt5 applications with relevant features
- IIO-based tools and utilities
- Signal processing implementations
- Plugin architectures in C++/Qt

**Use when:** Need a reference implementation to understand an approach.

### General Web Search
**Use for:**
- Algorithm documentation (DSP, FFT, filtering)
- C++17 patterns and best practices
- CMake integration patterns for third-party libraries
- Cross-platform build considerations

## How to Evaluate External Solutions

Before recommending any external library or tool, check:

### License Compatibility
Scopy is **LGPL-licensed**. Compatible licenses:
- LGPL (any version) — fully compatible
- MIT, BSD, Apache 2.0 — compatible
- GPL — **CAUTION**: only compatible if dynamically linked or the feature is optional
- Proprietary — **NOT compatible**

### Technical Requirements
- **Qt5 compatibility** — must work with Qt 5.x (not Qt 6-only)
- **Cross-platform** — must support Windows, Linux, macOS (Android is a plus)
- **CMake** — must integrate with CMake build system
- **C++17** — must compile with C++17 standard
- **No conflicting dependencies** — check against existing dependency list (libiio, QWT, Boost, GNU Radio, libsigrokdecode)

### Quality Indicators
- Actively maintained (commits in last 6 months)
- Has releases/tags (not just raw commits)
- Has tests
- Has documentation
- Used by other projects (stars, forks)
- Responsive maintainers (issue response time)

## How to Report External Findings

When presenting external solutions, use this format:

```markdown
### External Option: <library/tool name>
- **What it does:** <one sentence>
- **License:** <license name> — <compatible/caution/incompatible>
- **Platforms:** <list>
- **Source:** <URL>
- **Pros:**
  - <advantage 1>
  - <advantage 2>
- **Cons:**
  - <disadvantage 1>
  - <disadvantage 2>
- **Integration effort:** <low/medium/high> — <brief explanation>
- **Alternative:** <what we'd build if we don't use this>
```

## Research Workflow

1. **Understand the need** — What problem does the external solution solve?
2. **Search internally first** — Does Scopy already have something similar?
3. **Search externally** — Use the sources above
4. **Evaluate** — License, compatibility, quality
5. **Compare** — External solution vs building it ourselves
6. **Recommend** — Present options with trade-offs, include "build it ourselves" as an option

Always present "build it ourselves" alongside external options so the user can make an informed choice.
