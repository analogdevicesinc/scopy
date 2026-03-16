---
name: scopy-style-rules
description: Scopy styling system patterns including Style::setStyle, theme colors, section backgrounds, and widget styling. Auto-loads when applying styling to scopy widgets or sections.
---

# Scopy Style Rules

## Core Functions

```cpp
#include <style.h>

// Apply predefined styles
Style::setStyle(widget, style::properties::label::menuBig);
Style::setStyle(widget, style::properties::widget::border_interactive);
Style::setStyle(widget, style::properties::widget::basicBackground, true, true);

// Set background colors
Style::setBackgroundColor(widget, json::theme::background_primary);
Style::setBackgroundColor(widget, json::theme::background_subtle);

// Get theme values
QString iconPath = Style::getAttribute(json::theme::icon_theme_folder);
QColor color = Style::getColor(json::theme::content_error);
```

## Section Styling Pattern

```cpp
// Standard section
Style::setBackgroundColor(sectionWidget, json::theme::background_primary);
Style::setStyle(sectionWidget, style::properties::widget::border_interactive);

// Section title
Style::setStyle(titleLabel, style::properties::label::menuBig);
```

**Note**: Not every section needs `background_primary` — check reference implementations to see which sections use it vs inherit parent background.

## Key Style Properties

- Labels: `menuBig`, `menuMedium`, `menuSmall`, `subtle`, `deviceIcon`
- Buttons: `basicButton`, `blueGrayButton`, `squareIconButton`, `borderButton`
- Widgets: `border_interactive`, `basicBackground`
- Backgrounds: `background_primary`, `background_secondary`, `background_subtle`

## Collapsible Section (for advanced sub-tabs)

```cpp
#include <gui/widgets/menucollapsesection.h>

MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
    "Section Title", MenuCollapseSection::MHCW_ARROW,
    MenuCollapseSection::MHW_BASEWIDGET, parent);
section->contentLayout()->addWidget(widget);
```

## Rules

- Never hardcode colors — always use theme system
- Apply styles after widget creation
- Check widget validity before styling
- Always include `#include <style.h>`

## Full Reference

See `reference.md` in this directory for the complete style catalog.
