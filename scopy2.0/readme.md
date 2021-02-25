# Scopy

## Overview
Scopy is a software oscilloscope and signal analysis toolset.

## Supported Platforms
| OS | CI status |
| -- | --------- |
| Windows | windows ci badge |
| Ubuntu | ubuntu18 ci badge<br> ubuntu20 ci badge |
| MacOS | MacOS 10.15 ci badge<br> MacOS 11.0 ci badge |
| Kuiper | kuiper ci badge |
| Android<br> (TODO: version?) | android ci badge |

## Dependencies
| Dependency | Version | Affected targets |
| ---------- | ------- | ---------------- |
| CMake | >= 3.9.6 | build |
| Qt | >= 5.12 | gui, core, plugins, scopy |

## Directory Structure

| Directory | Description |
| --------- | ----------- |
| cmake | Cmake helper scripts for setting up the project |
| core | Core library containing |
| gui | Gui library containing the widgets that are used in Scopy
| plugins | Plugins directory containing all the tool packages that are available in Scopy with different hardware |
| app | Scopy executable |
