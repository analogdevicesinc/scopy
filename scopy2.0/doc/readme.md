# Coding Guide

## Overview 

This guide is here to aid in writting clean and consistent code throughout the application

## Standard
`c++11`

## File Naming
File name should be separated with underscore:
`my_custom_widget.*` with `.hpp` and `.cpp` as extensions

## Class Naming
```cpp
class MyCustomWidget {
        ...
};
```
with corresponding header my_custom_widget.hpp and my_custom_widget.cpp

## Function Members
Use camelCase
```cpp
// bad
void bad_naming_example();

// good
void goodNamingExample();
```

## Data Members
Use m_ as a prefix and camelCase
```cpp
// bad
int m_bad_member_name;

// good
int m_goodMemberName;
```

## Namespaces
Use the approapiate namespace given the location of the code
The first namespace should be `scopy` the second should be the current library/component
Example: for code in the gui library
```cpp
namespace scopy {
namespace gui {
        ...
} // namespace gui
} // namespace scopy
```

## Constants
Avoid defines
```cpp
// bad
#define MAX 300

// good
constexpr int MAX = 300;
static const int MAX = 300;
```

## QT

All of the above apply to Qt code as well.

When adding gui elements in .ui files make sure to follow the naming convention bellow

| Qt Class | Name Prefix |
| -------- | ------------------- |
| QWidget  | widget* |
| QHBoxLayout | hLayout* |
| QVBoxLayout | vLayout* |
| QPushButton | btn* |
| QComboBox | cmbBox* |
| QCheckBox | checkBox* |
| QLineEdit | lineEdit* |
| QLabel | lbl* |
| Line | line* |
