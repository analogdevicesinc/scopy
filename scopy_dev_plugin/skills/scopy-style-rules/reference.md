# Scopy Style Rules — Full Reference

## Label Styles

```cpp
Style::setStyle(label, style::properties::label::menuBig);      // Section titles
Style::setStyle(label, style::properties::label::menuMedium);    // Subsection titles
Style::setStyle(label, style::properties::label::menuSmall);     // Regular labels
Style::setStyle(label, style::properties::label::subtle);        // Helper text
Style::setStyle(label, style::properties::label::deviceIcon);    // Device icons
```

## Button Styles

```cpp
Style::setStyle(btn, style::properties::button::basicButton);       // Primary
Style::setStyle(btn, style::properties::button::blueGrayButton);    // Mode switching
Style::setStyle(btn, style::properties::button::squareIconButton);  // Icon buttons
Style::setStyle(btn, style::properties::button::borderButton);      // Border buttons
```

## Widget Container Styles

```cpp
// Section styling
Style::setBackgroundColor(widget, json::theme::background_primary);
Style::setStyle(widget, style::properties::widget::border_interactive);

// IIOWidget styling
Style::setStyle(widget, style::properties::widget::basicBackground, true, true);
```

## Background Colors

```cpp
json::theme::background_primary    // Main content areas
json::theme::background_secondary  // Secondary content
json::theme::background_subtle     // Less prominent areas
```

## Content/Status Colors

```cpp
json::theme::content_default   // Normal
json::theme::content_success   // Success
json::theme::content_error     // Error
json::theme::content_warning   // Warning
json::theme::content_busy      // Loading
```

## Interactive Colors

```cpp
json::theme::interactive_primary_idle
json::theme::interactive_primary_hover
json::theme::interactive_primary_pressed
json::theme::interactive_subtle_idle
json::theme::interactive_subtle_hover
json::theme::interactive_subtle_pressed
```

## Standard Layout Pattern

```cpp
void setupLayout()
{
    // Main tool template
    m_tool = new ToolTemplate(this);
    Style::setBackgroundColor(m_tool, json::theme::background_subtle);

    // Central widget
    QWidget *central = new QWidget(this);
    QVBoxLayout *centralLayout = new QVBoxLayout(central);

    // Sections
    QWidget *globalSection = createStyledSection("Global Settings", central);
    QWidget *rxSection = createStyledSection("RX Settings", central);
    centralLayout->addWidget(globalSection);
    centralLayout->addWidget(rxSection);
    centralLayout->addStretch(); // Push content to top

    // Scroll area
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setWidget(central);
    m_tool->addWidgetToCentralContainerHelper(scroll);
}
```

## Collapsible Section (MenuSectionCollapseWidget)

```cpp
#include <gui/widgets/menucollapsesection.h>

MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
    "Section Title",
    MenuCollapseSection::MHCW_ARROW,
    MenuCollapseSection::MHW_BASEWIDGET,
    parent);

// Add widgets to section content
section->contentLayout()->addWidget(widget);
```

Used in advanced tool sub-tabs. Handles styling and collapse/expand automatically.

## Rules

- Always use `#include <style.h>`
- Never hardcode colors — use theme system
- Apply styles AFTER widget creation
- Check widget validity before styling (`if (widget)`)
- Not all sections need `background_primary` — check reference implementations
- Use consistent spacing: `layout->setSpacing(10)`
- Add spacers at bottom: `layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding))`
- Add stretch to central layout: `centralLayout->addStretch()`
