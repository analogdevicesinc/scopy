# CMake helper scripts

# Overview

This folder contains helper cmake scripts that are used in the project

# Static Analysis
## Include-what-you-use

IWYU is a static analysis tool that tells you what to include, forward declare, or remove from a file.

To enable this tool the following option must be specified when configuring the project

```
-DENABLE_INCLUDE_WHAT_YOU_USE=on
```

# Code Style
## CMake Format

In order to keep our cmake files consistent, we use a tool called [cmake-format](https://github.com/cheshirekow/cmake_format)

The usage of this tool in the project for automatic style refactoring can be enabled by passing the option
```
-DENABLE_CMAKE_FORMAT=on
```
when configuring the project. After this the following command can be used in the build folder
```
make cmake-format
```


# Documentation
## Doxygen

In order to generate documentation the following option must be passed when configuring the project
```
-DENABLE_DOXYGEN=on
```
After this the following command can be used in the build folder to generate documentation
```
make doxygen
```
