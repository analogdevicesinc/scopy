# CMake helper scripts

# Overview

This folder contains helper cmake scripts that are used in the project

# CMake Format

In order to keep our cmake files consistent, we use a tool called [cmake-format](https://github.com/cheshirekow/cmake_format)

The usage of this tool in the project for automatic style refactoring can be enabled by passing the option
```
-DENABLE_CMAKE_FORMAT=on
```
when configuring the project. After this the following command can be used in the build folder
```
make cmake-format
```
