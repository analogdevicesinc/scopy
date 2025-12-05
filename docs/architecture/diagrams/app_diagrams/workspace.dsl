workspace "Scopy ADI" {

    model {

        // Users
        engineerUser = person "Engineer"
        student = person "Student"

        // External Systems
        gnuRadioEcosystem = softwareSystem "GNU Radio Ecosystem" {
            description "Signal processing backend for advanced operations"
            tags "System Dependency"
        }
        
        fileSystem = softwareSystem "File System" {
            description "Local and network storage for data, configurations, and logs"
            tags "System Dependency"
        }
        
        operatingSystem = softwareSystem "Operating System" {
            description "Linux, Windows, macOS providing hardware access and system services"
            tags "System Dependency"
        }
        
        iioFramework = softwareSystem "IIO Framework" {
            description "Linux Industrial I/O subsystem for hardware abstraction"
            tags "System Dependency"
        }
        
        adiHardware = softwareSystem "Hardware Devices" {
            description "Analog Devices evaluation boards and other devices"
            tags "System Dependency"
        } 

        iioEmu = softwareSystem "IIO Emulator (iio-emu)" {
            description "Hardware device emulation system providing virtualized IIO devices for development and testing"
            tags "System Dependency"
        }

        // Main System
         scopySystem = softwareSystem "Scopy" "Multi-functional software toolset with strong capabilities for signal analysis" {
            tags "Scopy System"
            // Single Main Container
            scopyApplication = container "Scopy Application" {
                description "Monolithic Qt-based desktop application"
                technology "C++, Qt, Qwt"
                tags "MainContainer"

                // INFRASTRUCTURE LAYER
                applicationCoordinator = component "Application Coordinator" {
                    description "Application startup/shutdown coordination, initialization sequences, and graceful termination handling"
                    tags "Infrastructure"
                    group "core"
                    properties {
                        "Application initialization" "QApplication setup, Qt attributes, logging, crash handling coordination (main.cpp)"
                        "Command line processing" "CmdLineHandler for argument parsing, script execution, device auto-connection (core/include/core/cmdlinehandler.h)"
                        "Application restart coordination" "ApplicationRestarter singleton for graceful restart handling (core/include/core/application_restarter.h)"
                        "Splash screen management" "ScopySplashscreen startup feedback and progress messaging (gui/include/gui/widgets/scopysplashscreen.h)"
                    }

                }

                crashReporting = component "Crash Reporting System" {
                    description "Crash detection, error report generation, and stack trace collection"
                    tags "Infrastructure"
                    group "core"
                }

                commandLineHandler = component "Command Line Handler" {
                    description "CLI argument parsing and command execution"
                    tags "Infrastructure"
                    group "core"
                }

                scriptingEngine = component "Scripting Engine" {
                    description "JavaScript runtime, script execution, automation interface, and programmatic API access"
                    tags "Infrastructure"
                    group "core"
                }
                
                translationService = component "Translation Service" {
                    description "Internationalization support and language pack management"
                    tags "Infrastructure"
                    group "core"
                }

                preferencesManagement = component "Preferences Management" {
                    description "Centralized configuration storage, user preferences, plugin settings, and persistent application state"
                    tags "Infrastructure"
                    group "core"
                }

                loggingSystem = component "Logging System" {
                    description "Centralized logging, debug output, error tracking, benchmark, and diagnostic information collection"
                    tags "Infrastructure"
                    group "core"
                }

                // CORE BUSINESS LOGIC
                deviceManagement = component "Device Management" {
                    description "Device discovery, connection lifecycle, hardware abstraction, and unified device interface"
                    tags "CoreBusiness"
                    group "core"
                    properties {
                        "Device management" "Device creation, connection, disconnection, restart, reload with state tracking (core/include/core/devicemanager.h)"
                        "Device abstraction" "DeviceImpl and Device classes providing unified hardware interface (core/include/core/device.h; core/include/core/deviceimpl.h)"
                    }
                }

                packageManager = component "Package Manager" {
                    description "Manages packages installation and distribution"
                    tags "CoreBusiness"
                    group "pkg-manager"
                }

                pluginSystem = component "Plugin System" {
                    description "Dynamic plugin loading, lifecycle management, dependency resolution, and plugin architecture foundation"
                    tags "CoreBusiness"
                    group "core"
                    properties {
                        "Plugin repository" "PluginRepository singleton for plugin management, metadata handling, plugin discovery (core/include/core/pluginrepository.h)"
                        "Plugin manager" "PluginManager for dynamic loading, sorting, filtering, and lifecycle control (core/include/core/pluginmanager.h)"
                        "Plugin interface" "Plugin base interface defining compatible(), onConnect(), onDisconnect() and other contracts (pluginbase/include/pluginbase/plugin.h)"
                    }
                }
                
                // Plugin System components
                pluginRepository = component "Plugin Repository" {
                    description "Storage and management of available plugins and their metadata"                                                    
                    tags "PluginSystem"                                          
                    group "core"
                }
                pluginManager = component "Plugin Manager" {                 
                    description "Dynamic loading and instantiation of plugin libraries"                                                              
                    tags "PluginSystem"                                         
                    group "core" 
                }
                pluginInterface = component "Plugin Interface" {           
                    description "Common API contract that all plugins must implement"                                                                    
                    tags "PluginSystem" 
                    group "pluginbase"                                         
                }  
                pluginManager -> pluginRepository "queries available plugins"
                pluginManager -> pluginInterface "instantiates plugins"

                // UI AND PRESENTATION LAYER
                uiFramework = component "UI Framework" {
                    description "Common UI widgets, plot widgets, style management"
                    tags "Presentation"
                    group "gui"
                    properties {
                        "Common components" "Menu section widgets, collapse headers, custom controls for patameters, enhanced input widgets, etc. (gui/include/gui; gui/include/gui/widgets)"
                        "Plot widgets" "Plotting widgets, measurement displays, signal visualization (gui/include/gui)"
                        "Style management" "QSS-based styling with dynamic theme switching (gui/style)"
                    }
                }

                windowManagement = component "Window Management" {
                    description "Main window coordination, detached tool windows, and workspace management"
                    tags "Presentation"
                    group "gui"
                    properties {
                        "Main window coordination" "ScopyMainWindow lifecycle, application initialization, event handling (core/include/core/scopymainwindow.h)"
                        "Detached tool management" "DetachedToolWindowManager for separate tool windows, multi-monitor support (core/include/core/detachedtoolwindowmanager.h)"
                        "Advanced docking system" "KDDockWidgets backend for professional docking layouts (gui/include/gui/docking)"
                    }
                }

                toolManager = component "Tool Manager" {
                    description "Tool registration, menu organization, tool lifecycle coordination, and unified tool interface"
                    tags "Presentation"
                    group "core"
                }

                statusBar = component "Status Bar" {
                    description "Real-time status display, device connectivity indicators, operation progress, and system notifications"
                    tags "Presentation"
                    group "gui"
                }

                applicationPages = component "Application Pages" {
                    description "Page navigation, home page, device browser, preferences pages, packages page, and application flow control"
                    tags "Presentation"
                    group "core"
                    properties {
                        "Tool stack management" "ToolStack container extending MapStackedWidget for key-based page switching (core/include/core/toolstack.h)"
                        "Preferences integration" "ScopyPreferencesPage, settings management, configuration dialogs (core/include/core/scopypreferencespage.h)"
                        "About and info pages" "ScopyAboutPage, version information, system details (core/include/core/scopyaboutpage.h)"
                        "Home page system" "ScopyHomePage for device scanning, connection management, device browser (core/include/core)"
                        "Navigation coordination" "Tool selection flow, page switching, menu state management (core/include/core)"
                    }
                }

                // SUPPORTING LIBRARIES
                iioUtilities = component "IIO Utilities" {
                    description "Industrial I/O communication, command queuing, and connection management"
                    tags "Library"
                    group "iio-util"
                }

                iioWidgets = component "IIO Widgets" {
                    description "Specialized UI components for IIO devices, parameter controls, and device-specific interfaces"
                    tags "Library"
                    group "iio-widgets"
                }

                gnuRadioWidgets = component "GNU Radio Utilities" {
                    description "GNU Radio specific components and signal processing controls"
                    tags "Library"
                    group "gr-util"
                }

                commonUtilities = component "Common Utilities" {
                    description "Shared utilities, helper functions, data structures, and cross-cutting functionality"
                    tags "Library"
                    group "common"
                }
                
                // COMPONENT RELATIONSHIPS

                // Infrastructure Dependencies
  
                // Application Coordinator orchestrates system initialization
                applicationCoordinator -> preferencesManagement "loads application settings"
                applicationCoordinator -> loggingSystem "initializes logging"
                applicationCoordinator -> crashReporting "sets up crash handlers"
                applicationCoordinator -> translationService "initializes localization"
                applicationCoordinator -> deviceManagement "initializes device system"
                applicationCoordinator -> pluginSystem "initializes plugin system"
                applicationCoordinator -> packageManager "initializes package management"
                applicationCoordinator -> windowManagement "initializes main window"


                // Command Line Handler processes startup arguments
                commandLineHandler -> preferencesManagement "overrides default settings"
                commandLineHandler -> loggingSystem "configures log verbosity"
                commandLineHandler -> applicationCoordinator "triggers startup sequence"
                commandLineHandler -> scriptingEngine "executes script files"
                
                scriptingEngine -> applicationCoordinator "accesses application services"

                // Core Business Logic Dependencies
                deviceManagement -> preferencesManagement "reads device preferences"
                deviceManagement -> loggingSystem "logs device operations"
                deviceManagement ->  pluginSystem "loads device specific plugins"

                pluginSystem -> packageManager "manages plugins from packages"
                pluginSystem -> preferencesManagement "reads plugin configurations"
                pluginSystem -> loggingSystem "logs plugin operations"
                pluginSystem -> iioUtilities "uses for device communication"
                pluginSystem -> gnuRadioWidgets "specific plugins use processing services"
                pluginSystem -> iioWidgets "specific plugins use"

                // UI and Presentation Dependencies
                windowManagement -> uiFramework "uses common UI components"
                windowManagement -> preferencesManagement "persists window state"
                windowManagement -> applicationPages "manages page containers"
                windowManagement -> toolManager "manages tool windows"

                toolManager -> pluginSystem "coordinates available plugin tools"
                toolManager -> uiFramework "uses UI components"

                statusBar -> deviceManagement "subscribes to device status"
                statusBar -> pluginSystem "receives plugin notifications"
                statusBar -> uiFramework "uses status widgets"

                applicationPages -> uiFramework "uses page components"
                applicationPages -> preferencesManagement "reads page preferences"
                
                uiFramework -> packageManager "manages styles elements from packages"

                // Library Dependencies
                iioUtilities -> commonUtilities "uses shared utilities"
                iioWidgets -> iioUtilities "uses IIO communication"
                iioWidgets -> uiFramework "extends UI framework"

            }

            // GENERIC PLUGINS PACKAGE
            genericPluginsPackage = container "Generic Plugins Package" {
                description "Standard IIO device plugins for common functionality"
                technology "C++, Qt"

                adcPlugin = component "ADC Plugin" {
                    description "The ADC plugin is used to interface with IIO ADCs that implement an IIO buffer mechanism"
                    tags "Plugin"
                }
                dacPlugin = component "DAC Plugin" {
                    description "The DAC plugin is used to interface with IIO DACs that implement the IIO buffer mechanism or a DDS mechanism"
                    tags "Plugin"
                }
                dataloggerPlugin = component "DataLogger Plugin" {
                    description "Used to monitor and log data"
                    tags "Plugin"
                }
                debuggerPlugin = component "Debugger Plugin" {
                    description "The debugger plugin is used to examine IIO contexts and modify individual IIO attributes, as well as examining the structure of an IIO context"
                    tags "Plugin"
                } 
                jesdPlugin = component "JESD Status Plugin" {
                    description "The JESD Status utility plugin provides a graphical interface to monitor the status of JESD204 in Scopy"
                    tags "Plugin"
                }
                regmapPlugin = component "Register Map Plugin" {
                    description "The Register Map allow access to reading and writing registers for devices connected to Scopy"
                    tags "Plugin"
                }
                
                adcPlugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
                adcPlugin -> iioWidgets "use"
                adcPlugin -> gnuRadioWidgets "use"
                
                dacPlugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
                dacPlugin -> iioWidgets "use"
                
                dataloggerPlugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
                dataloggerPlugin -> iioWidgets "use"
                
                debuggerPlugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
                debuggerPlugin -> iioWidgets "use"

                jesdPlugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
                regmapPlugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
            }

            // M2K PACKAGE
            m2kPackage = container "M2K Package" {
                description "ADALM2000 device support package"
                technology "C++, Qt"

                m2kPlugin = component "M2K Plugin" {
                    description "Plugin for ADALM2000 (M2K)"
                    tags "Plugin"
                }

                // Relations
                m2kPlugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
                m2kPlugin -> gnuRadioWidgets "use"
            }

            // SWIOT PACKAGE
            swiotPackage = container "SWIOT Package" {
                description "AD-SWIOT1L-SL platform support package"
                technology "C++, Qt"

                swiotPlugin = component "SWIOT1L Plugin" {
                    description "The Scopy AD-SWIOT1L-SL plugin is responsible with the operation and control of the AD-SWIOT1L-SL platform"
                    tags "Plugin"
                }

                // Relations
                swiotPlugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
                swiotPlugin -> iioWidgets "use"
            }

            // AD936X PACKAGE
            ad936xPackage = container "AD936X Package" {
                description "AD936X transceiver support package"
                technology "C++, Qt"

                ad936xPlugin = component "AD936X Plugin" {
                    description "The AD936x plugins for Scopy enable integration and control of AD936x-based devices within the Scopy software environment"
                    tags "Plugin"
                }
            
                // Relations
                ad936xPlugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
                ad936xPlugin -> iioWidgets "use"  

            }

            // APOLLO AD9084 PACKAGE
            apolloAd9084Package = container "Apollo AD9084 Package" {
                description "Apollo MXFE QUAD AD9084 support package"
                technology "C++, Qt"

                ad9084Plugin = component "AD9084 Plugin" {
                    description "Apollo MXFE QUAD AD9084 interaction"
                    tags "Plugin"
                }

                // Relations
                ad9084Plugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
                ad9084Plugin -> iioWidgets "use"
            }

            // POWER QUALITY MONITOR PACKAGE
            pqmonPackage = container "PQMon Package" {
                description "Power Quality Monitor support package"
                technology "C++, Qt"

                pqmPlugin = component "PQMON Plugin" {
                    description "Power Quality Monitor plugin"
                    tags "Plugin"
                }
                pqmPlugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
            }

            // IMU PACKAGE
            imuPackage = container "IMU Package" {
                description "IMU Evaluation Software Package"
                technology "C++, Qt"

                imuAnalyzerPlugin = component "IMU Analyzer Plugin" {
                    description "IMU evaluation and analysis functionality"
                    tags "Plugin"
                }

                // Relations
                imuAnalyzerPlugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }
                imuAnalyzerPlugin -> iioWidgets "use"
            }

            // ADRV9002 PACKAGE
            adrv9002Package = container "ADRV9002 Package" {
                description "ADRV9002 Jupiter transceiver support package"
                technology "C++, Qt"

                adrv9002Plugin = component "ADRV9002 Plugin" {
                    description "The ADRV9002 (Jupiter) plugin provides a comprehensive interface for controlling and configuring the ADRV9002 dual-channel RF transceiver"
                    tags "Plugin"
                }
                adrv9002Plugin -> pluginInterface "implements" {
                    tags "PluginImplementation"
                }

            }


            // User Relationships
            engineerUser -> scopyApplication "uses GUI for device control and analysis"
            student -> scopyApplication "uses for learning and experiments"

            // External Container Relationships
            scopyApplication -> iioFramework "sends commands and receives data via"
            scopyApplication -> gnuRadioEcosystem "uses for signal processing and DSP operations"
            scopyApplication -> fileSystem "read/write data files and configurations"
            scopyApplication -> operatingSystem "runs on cross-platform OS"
            scopyApplication -> iioEmu "launches and communicates with via TCP/IP"
            scopyApplication -> genericPluginsPackage "loads resources from"
            scopyApplication -> m2kPackage "loads resources from"
            scopyApplication -> swiotPackage "loads resources from"
            scopyApplication -> ad936xPackage "loads resources from"
            scopyApplication -> apolloAd9084Package "loads resources from"
            scopyApplication -> pqmonPackage "loads resources from"
            scopyApplication -> imuPackage "loads resources from"
            scopyApplication -> adrv9002Package "loads resources from"
        }        
        
        iioFramework -> adiHardware "abstracts hardware communication to"
        
    }
    
    views {

        // SYSTEM CONTEXT VIEW
        systemContext scopySystem "SystemContext" {
            include *
            include adiHardware
            title "Scopy"
            description "High-level view showing Scopy's interactions with users and external systems"
        }

        // CONTAINER VIEW
        container scopySystem "ContainerView" {
            include *
            exclude "relationship.tag==PluginImplementation"
            exclude "element.tag==System Dependency"
            title "Scopy Architecture"
            description "Container relationships showing how core application loads plugins from packages"
        }
        
        // Infrastructure View
        component scopyApplication "InfrastructureComponents" {
            include applicationCoordinator
            include crashReporting
            include translationService
            include scriptingEngine
            include preferencesManagement
            include loggingSystem
            title "Infrastructure Components"
            description "Core application infrastructure"
        }

        // Core Business View
        component scopyApplication "CoreBusinessComponents" {
            include deviceManagement
            include pluginSystem
            include packageManager
            title "Core Business Components"
            description "Main business logic components"
        }

        // UI Layer View
        component scopyApplication "UIComponents" {
            include uiFramework
            include windowManagement
            include toolManager
            include statusBar
            include applicationPages
            title "UI Components"
            description "User interface layer components"
        }

        // Library View
        component scopyApplication "LibraryComponents" {
            include iioUtilities
            include iioWidgets
            include gnuRadioWidgets
            include commonUtilities
            title "Supporting Libraries"
            description "Reusable library components"
        }

        // Overall View
        component scopyApplication "CoreComponents" {
            include applicationCoordinator
            include deviceManagement
            include pluginSystem
            include uiFramework
            include windowManagement
            include iioUtilities
            include packageManager
            include toolManager
            include gnuRadioWidgets
            include iioWidgets
            title "Components Overview"
            description "Key components and their relationships"
        }

        // Plugin View
        component scopyApplication "PluginComponents" {
            include pluginRepository
            include pluginManager
            include pluginInterface

            title "Plugin Components"
            description "Plugin system components"
        }

        // Generic-plugins Package
        component genericPluginsPackage "Generic-pluginsComponents" {
            include *
            exclude scopyApplication
            include pluginInterface
            include iioWidgets
            include gnuRadioWidgets
            title "Generic-plugins Package"
            description "The plugins from generic-plugins package"
        }

        // M2K Package
        component m2kPackage "M2KComponents" {
            include *
            exclude scopyApplication
            include pluginInterface
            include gnuRadioWidgets
            title "M2K Package"
            description "The plugins from M2K package"
        }

        // SWIOT Package
        component swiotPackage "SWIOTComponents" {
            include *
            exclude scopyApplication
            include pluginInterface
            include iioWidgets
            title "SWIOT Package"
            description "The plugins from SWIOT package"
        }

        // AD936X Package
        component ad936xPackage "AD936XComponents" {
            include *
            exclude scopyApplication
            include pluginInterface
            include iioWidgets
            title "AD936X Package"
            description "The plugins from AD936X package"
        }

        // Apollo AD9084 Package
        component apolloAd9084Package "Apollo-AD9084Components" {
            include *
            exclude scopyApplication
            include pluginInterface
            include iioWidgets
            title "Apollo AD9084 Package"
            description "The plugins from Apollo AD9084 package"
        }

        // PQMon Package
        component pqmonPackage "PQMonComponents" {
            include *
            exclude scopyApplication
            include pluginInterface
            title "PQMon Package"
            description "The plugins from PQMon package"
        }

        // IMU Package
        component imuPackage "IMUComponents" {
            include *
            exclude scopyApplication
            include pluginInterface
            include iioWidgets
            title "IMU Package"
            description "The plugins from IMU package"
        }

        // ADRV9002 Package
        component adrv9002Package "ADRV9002Components" {
            include *
            exclude scopyApplication
            include pluginInterface
            title "ADRV9002 Package"
            description "The plugins from ADRV9002 package"
        }


        styles {            
            element "Scopy System" {
                background "#0077b6"
                color "White"
            }
            
            element "Container" {
                background "#0077b6"
                color "White"
            }
            
            element "Person" {
                shape "Person"
                background "#023047"
                color "White"
            }
            
            element "Infrastructure" {
                background #ff6b6b
                color "Black"
            }

            element "CoreBusiness" {
                background #4ecdc4
                color "Black"
            }

            element "Presentation" {
                background #f9ca24
                color "Black"
            }

            element "Library" {
                background #6c5ce7
                color "Black"
            }
            
            element "PluginSystem" {
                background "#00897b"
                color "Black"
            }

            element "Plugin" {
                background "#ffb86b"
                color "Black"
            }
            
            element "System Dependency" {
                background "#e0e0e0"
                color "#666666"
                opacity 50
            }
        }
    }

}
