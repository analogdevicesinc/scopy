.. _architecture:

Software Architecture
=====================

This section provides comprehensive documentation of Scopy's software architecture
using interactive diagrams. The architecture is documented using Structurizr DSL
and includes both application and deployment views.

Interactive Diagram Workspaces 
------------------------------

Application Architecture
~~~~~~~~~~~~~~~~~~~~~~~~

The application architecture workspace provides detailed views of Scopy's internal
structure, including:

* **Infrastructure Layer**: Application coordination, preferences, logging, and crash reporting
* **Core Business Logic**: Device management, plugin system, and package management
* **UI Framework**: Window management, common widgets, and presentation components
* **Plugin Packages**: M2K, SWIOT, AD936X, ADRV9002, IMU, PQMon and generic plugins
* **Supporting Libraries**: IIO utilities, GNU Radio widgets, and common utilities

.. tip::

  **View Interactive Application Architecture**

  `Open Application Architecture Diagrams <diagrams/app-architecture/index.html>`_

  Explore 15+ component views showing relationships between infrastructure,
  core business logic, UI components, and plugin packages.

Deployment Architecture
~~~~~~~~~~~~~~~~~~~~~~~

The deployment architecture workspace documents the CI/CD pipeline and
multi-platform build process:

* **Build Infrastructure**: GitHub Actions and Azure DevOps workflows
* **Docker Environments**: Pre-configured build containers for each platform
* **Platform Support**: Windows (MinGW), Linux (Flatpak, AppImage), macOS (DMG), ARM platforms
* **Artifact Distribution**: Automated package creation and GitHub Releases distribution

.. tip::

  **View Interactive Deployment Architecture**

  `Open Deployment Architecture Diagrams <diagrams/deployment-architecture/index.html>`_

  Understand the complete build and deployment pipeline from source code to distributed packages across multiple platforms.

System Context
--------------

Overview
~~~~~~~~

Scopy is a multi-functional software toolset with strong capabilities for
signal analysis, designed to work with Analog Devices hardware evaluation boards.

External Systems
~~~~~~~~~~~~~~~~

**Hardware Integration**

* **ADI Hardware Devices**: ADALM2000, AD936X, ADRV9002, AD-SWIOT1L-SL, and other evaluation boards
* **IIO Framework**: Linux Industrial I/O subsystem providing hardware abstraction layer

**Software Ecosystem**

* **GNU Radio**: Signal processing backend for advanced DSP operations
* **Operating Systems**: Cross-platform support for Linux, Windows, and macOS
* **File System**: Local and network storage for data, configurations, and logs

Core Components
---------------

Application Infrastructure
~~~~~~~~~~~~~~~~~~~~~~~~~~

**Application Coordinator**
  Central orchestration of application startup, shutdown, and lifecycle management

**Device Management**
  Unified interface for hardware device discovery, connection, and communication

**Plugin System**
  Dynamic loading and management of device-specific functionality through packages

**Package Manager**
  Installation and distribution system for plugin packages

**Preferences Management**
  Centralized configuration storage and user settings persistence

Plugin Architecture
~~~~~~~~~~~~~~~~~~~

Scopy uses a modular plugin-based architecture where functionality is organized
into specialized packages:

**Generic Plugins**
  * ADC Plugin - Interface with IIO ADCs using buffer mechanisms
  * DAC Plugin - Interface with IIO DACs using buffer or DDS mechanisms
  * DataLogger Plugin - Data monitoring and logging capabilities
  * Debugger Plugin - IIO context examination and attribute modification
  * JESD Status Plugin - JESD204 interface status monitoring
  * Register Map Plugin - Direct device register access

**Device-Specific Packages**
  * **M2K Package**: ADALM2000 (M2K) device support with oscilloscope, signal generator, logic analyzer
  * **SWIOT Package**: AD-SWIOT1L-SL platform support for industrial IoT applications
  * **AD936X Package**: AD936X transceiver family support for RF applications
  * **ADRV9002 Package**: ADRV9002 Jupiter transceiver with dual-channel RF capabilities
  * **IMU Package**: Inertial measurement unit evaluation and analysis tools
  * **PQMon Package**: Power quality monitoring and analysis capabilities

CI/CD Pipeline
--------------

Build Infrastructure
~~~~~~~~~~~~~~~~~~~~

**GitHub Actions Workflows**
  * Windows MinGW builds with MSYS2 environment
  * Linux Flatpak sandboxed package creation
  * Linux AppImage portable application builds
  * ARM64/ARMHF cross-compilation for embedded platforms
  * Ubuntu development builds for testing

**Azure DevOps**
  * macOS DMG package creation using Homebrew dependencies

**Docker Infrastructure**
  * Pre-configured build environments for each target platform
  * Consistent dependency management across build systems
  * Reproducible builds with version-controlled containers

Deployment Targets
~~~~~~~~~~~~~~~~~~

**Platform Coverage**
  * Windows: Setup executable (.exe) installer
  * Linux: Flatpak packages (.flatpak) and AppImage executables (.AppImage)
  * macOS: Disk image packages (.dmg)
  * ARM platforms: Cross-compiled AppImage executables for ARM64 and ARMHF

**Distribution**
  * Automated artifact publishing to GitHub Releases
  * Version-controlled release management
  * Multi-platform simultaneous deployment

Source Documentation
--------------------

The architecture diagrams are generated from version-controlled Structurizr DSL files:

* ``docs/architecture/diagrams/app_diagrams/workspace.json`` - Application architecture definition
* ``docs/architecture/diagrams/deployment_diagrams/workspace.json`` - Deployment architecture definition

These files are automatically processed during documentation builds to generate
the interactive diagram websites accessible through the links above.