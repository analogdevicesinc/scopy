---
name: scopy-doc-format
description: RST documentation conventions for Scopy plugins including heading styles, toctree syntax, image placeholders, and attribute table format. Auto-loads when writing RST documentation for Scopy plugins.
---

# Scopy RST Documentation Conventions

## Heading Hierarchy

```rst
Top-level title (80 = characters)
================================================================================

Section heading (dashes)
---------------------------------

Subsection heading (tildes)
~~~~~~~~~~~~~~~~~~
```

The underline must be **at least as long** as the heading text.

## Cross-Reference Labels

```rst
.. _<plugin>:           # Main doc file
.. _<plugin>_index:     # Index file
.. _<plugin>_advanced:  # Advanced doc file
```

Labels must be unique across the entire documentation tree.

## Toctree Syntax

```rst
.. toctree::
   :maxdepth: 2

   <plugin>
   <plugin>_advanced
```

- No `.rst` extension in toctree entries
- Entries must match actual file names exactly

## Image Placeholder Format

Do NOT use `.. figure::` or `.. image::` directives. Use this comment format:

```rst
.. AN_IMAGE_WILL_BE_HERE
   Screenshot: <exact description of what to capture>
   e.g. "The RX Chain section showing hardware gain and sampling rate controls"
```

Place one placeholder:
- At the top of the main RST file (full plugin view)
- Per major section (zoomed in on that panel)
- For the advanced tab (if it exists)

## Attribute Documentation Format

```rst
- **Attribute Name:** One sentence description of what this attribute controls.
- **Another Attribute:** Description without wiki links.
```

- Use bullet list with **bold name** followed by colon
- Descriptions are self-contained — no wiki links
- 2-space indentation for RST lists (no tabs)

## Index File Structure

```rst
.. _<plugin>_index:

<Device Name>
========================================

Overview
--------
<One paragraph about the plugin.>

Features
--------
- **Feature 1:** Description

Supported Devices
-------------------
- **<Device Family>** (list variants)

Getting Started
---------------
**Prerequisites**
- Hardware requirement
- Scopy version X.Y or later

.. toctree::
   :maxdepth: 2

   <plugin>
   <plugin>_advanced
```

## Top-Level Toctree Update

When adding a new plugin, update `scopy/docs/plugins/index.rst`:
- Add to the bullet list in the appropriate category
- Add to the toctree block at the bottom
- Insert in alphabetical order within the group
