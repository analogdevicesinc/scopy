workspace "Scopy Deployment Architecture" "CI/CD and deployment architecture for Scopy multi-platform builds" {

    model {
        # External systems
        github = softwareSystem "GitHub" "Source code repository" "External"
        dockerhub = softwareSystem "Docker Hub" "Container image registry" "External"
        artifactStore = softwareSystem "GitHub Releases" "Artifact storage and distribution" "External"

        # Main software system
        scopy = softwareSystem "Scopy" "Signal analysis and device control software for ADI hardware" {
            # Core application
            scopyApp = container "Scopy Application" "Main Qt-based GUI application" "C++/Qt"

            # Dependencies
            libiio = container "libiio" "Industrial I/O library for ADI hardware" "C"
            gnuradio = container "GNU Radio" "Signal processing framework" "C++/Python"
            libm2k = container "libm2k" "ADALM2000 support library" "C++"
            qwt = container "Qwt" "Technical plotting widgets" "C++/Qt"
            libsigrok = container "libsigrokdecode" "Protocol decoder library" "C"
        }

        # CI/CD Platform 
        cicdPlatform = softwareSystem "CI/CD Platform" "CI/CD platform orchestration layer" {
            # GitHub Actions Platform
            githubActions = container "GitHub Actions" "GitHub-hosted CI/CD runners and workflow execution" "GitHub Platform" 
            azureDevOps = container "Azure DevOps" "Microsoft cloud CI/CD platform" "Azure Platform"
        }

        # CI/CD Infrastructure
        cicd = softwareSystem "CI/CD Infrastructure" "Continuous integration and deployment system" {

            # GitHub Actions workflows
            windowsWorkflow = container "Windows MinGW Workflow" "Builds Windows installer" "GitHub Actions"
            linuxFlatpakWorkflow = container "Linux Flatpak Workflow" "Builds Flatpak package" "GitHub Actions"
            linuxAppImageWorkflow = container "Linux AppImage Workflow" "Builds AppImage packages" "GitHub Actions"
            arm64Workflow = container "ARM64 Build Workflow" "Cross-compiles for ARM64 platforms" "GitHub Actions"
            armhfWorkflow = container "ARMHF Build Workflow" "Cross-compiles for ARMHF platforms" "GitHub Actions"
            ubuntuWorkflow = container "Ubuntu Workflow" "Build on ubuntu" "GitHub Actions"
            macosWorkflow = container "macOS Build Workflow" "Builds macOS DMG package" "Azure DevOps"
        }

        # Docker Build Environments
        dockerInfra = softwareSystem "Docker Build Infrastructure" "Pre-configured build environments" {
            mingwContainer = container "mingw64 container" "Windows build environment with MSYS2" "cristianbindea/scopy2-mingw64"
            flatpakContainer = container "flatpak container" "Linux Flatpak build environment" "cristianbindea/scopy2-flatpak"
            x86Container = container "x86_64-appimage container" "x86_64 AppImage build environment" "cristianbindea/scopy2-x86_64-appimage"
            arm64Container = container "arm64-appimage container" "ARM64 cross-compilation environment" "cristianbindea/scopy2-arm64-appimage"
            armhfContainer = container "armhf-appimage container" "ARMHF cross-compilation environment" "cristianbindea/scopy2-armhf-appimage"
            ubuntuContainer = container "ubuntu-22 container" "Development build environment" "cristianbindea/scopy2-ubuntu22"
        }

        # Build Scripts and Configuration
        buildScripts = softwareSystem "Build Scripts" "Platform-specific build automation" {
            windowsScript = container "Windows Build Script" "MSYS2/MinGW build process" "ci/windows/windows_build_process.sh"
            flatpakScript = container "Flatpak Build Script" "Sandboxed Flatpak build" "ci/flatpak/flatpak_build_process.sh"
            appimageScript = container "AppImage Build Script" "AppImage packaging process" "ci/x86_64/x86-64_appimage_process.sh"
            arm64Script = container "ARM64 Build Script" "Cross-compilation for ARM64" "ci/arm64/arm64_build_process.sh"
            armhfScript = container "ARMHF Build Script" "Cross-compilation for ARMHF" "ci/armhf/armhf_build_process.sh"
            ubuntuScript = container "Ubuntu Build Script" "Ubuntu development builds" "ci/ubuntu/ubuntu_build_process.sh"
            macosScript = container "macOS Build Script" "Homebrew-based macOS build" "ci/macOS/build_azure_macos.sh"
        }

        # Deployment targets
        deploymentTargets = softwareSystem "Deployment Targets" "Final deployment destinations" {
            windowsInstaller = container "Windows Installer" "Scopy Windows setup executable" ".exe"
            linuxFlatpak = container "Linux Flatpak" "Sandboxed Linux package" ".flatpak"
            linuxAppImage = container "Linux AppImage" "Portable Linux application" ".AppImage"
            arm64AppImage = container "ARM64 AppImage" "ARM64 portable application" ".AppImage"
            armhfAppImage = container "ARMHF AppImage" "ARMHF portable application" ".AppImage"
            ubuntuPackage = container "Ubuntu build" "Development build" "test build"
            macosDMG = container "macOS DMG" "macOS disk image package" ".dmg"
        }

        # GitHub triggers individual workflows
        github -> githubActions "Triggers on push/PR"
        github -> azureDevOps "Triggers on push/PR"
        githubActions -> windowsWorkflow "Runs"
        githubActions -> linuxFlatpakWorkflow "Runs"
        githubActions -> linuxAppImageWorkflow "Runs"
        githubActions -> arm64Workflow "Runs"
        githubActions -> armhfWorkflow "Runs"
        githubActions -> ubuntuWorkflow "Runs"
        azureDevOps -> macosWorkflow "Runs"


        # Workflow relationships
        cicd -> dockerhub "Pulls container images"
        windowsWorkflow -> dockerhub "Pulls MinGW container"
        linuxFlatpakWorkflow -> dockerhub "Pulls Flatpak container"
        linuxAppImageWorkflow -> dockerhub "Pulls AppImage container"
        arm64Workflow -> dockerhub "Pulls ARM64 container"
        armhfWorkflow -> dockerhub "Pulls ARMHF container"
        ubuntuWorkflow -> dockerhub "Pulls Ubuntu container"
        dockerInfra -> buildScripts "Executes"

        windowsWorkflow -> mingwContainer "Uses"
        linuxFlatpakWorkflow -> flatpakContainer "Uses"
        linuxAppImageWorkflow -> x86Container "Uses"
        arm64Workflow -> arm64Container "Uses"
        armhfWorkflow -> armhfContainer "Uses"
        ubuntuWorkflow -> ubuntuContainer "Uses"


        mingwContainer -> windowsScript "Executes"
        flatpakContainer -> flatpakScript "Executes"
        x86Container -> appimageScript "Executes"
        arm64Container -> arm64Script "Executes"
        armhfContainer -> armhfScript "Executes"
        ubuntuContainer -> ubuntuScript "Executes"
       
        # Build script execution
        macosWorkflow -> macosScript "Executes" {
            tag "workflow"
        }


        # Build outputs
        ubuntuScript -> ubuntuPackage "Generates"
        windowsScript -> windowsInstaller "Generates"
        flatpakScript -> linuxFlatpak "Generates"
        appimageScript -> linuxAppImage "Generates"
        arm64Script -> arm64AppImage "Generates"
        armhfScript -> armhfAppImage "Generates"
        macosScript -> macosDMG "Generates"

        # Artifact storage
        windowsInstaller -> artifactStore "Uploads to (continuous release)"
        linuxFlatpak -> artifactStore "Uploads to (continuous release)"
        linuxAppImage -> artifactStore "Uploads to (artifacts)"
        arm64AppImage -> artifactStore "Uploads to (artifacts)"
        armhfAppImage -> artifactStore "Uploads to (artifacts)"
        macosDMG -> artifactStore "Uploads to (artifacts)"

        # Dependencies within Scopy
        scopyApp -> libiio "Uses for hardware communication"
        scopyApp -> gnuradio "Uses for signal processing"
        scopyApp -> libm2k "Uses for ADALM2000 support"
        scopyApp -> qwt "Uses for plotting"
        scopyApp -> libsigrok "Uses for protocol decoding"


        # Users
        developer = person "Developer" "Scopy developer"
        endUser = person "End User" "Engineer using Scopy for signal analysis"

        developer -> github "Commits code"
        developer -> scopy "Develops"
        endUser -> artifactStore "Downloads"

        # Deployment environments
        buildEnvironment = deploymentEnvironment "Multi-Platform Build Environment" {
            deploymentNode "GitHub Actions Infrastructure" "GitHub-hosted CI/CD platform" "GitHub Actions" {
                deploymentNode "Windows Runner" "Windows 2022" "Microsoft Windows" {
                    deploymentNode "MinGW Docker Container" "cristianbindea/scopy2-mingw64" "MSYS2/MinGW64" {
                        containerInstance windowsScript
                        containerInstance scopyApp "Windows Build"
                    }
                }
                deploymentNode "Linux Runner" "Ubuntu 22.04" "Ubuntu Linux" {
                    deploymentNode "Flatpak Container" "cristianbindea/scopy2-flatpak" "Flatpak Runtime" {
                        containerInstance flatpakScript
                        containerInstance scopyApp "Flatpak Build"
                    }
                    deploymentNode "AppImage Container" "cristianbindea/scopy2-x86_64-appimage" "Ubuntu Container" {
                        containerInstance appimageScript
                        containerInstance scopyApp "AppImage Build"
                    }
                    deploymentNode "Ubuntu Container" "cristianbindea/scopy2-ubuntu" "Development Environment" {
                        containerInstance ubuntuScript
                        containerInstance scopyApp "Ubuntu Build"
                    }
                    deploymentNode "ARM64 Container" "cristianbindea/scopy2-arm64-appimage" "Cross-compilation" {
                        deploymentNode "ARM64 Sysroot" "aarch64-linux-gnu" "Chroot Environment" {
                            containerInstance arm64Script "ARM64 Build"
                            containerInstance scopyApp "ARM64 Binary"
                        }
                    }
                    deploymentNode "ARMHF Container" "cristianbindea/scopy2-armhf-appimage" "Cross-compilation" {
                        deploymentNode "ARMHF Sysroot" "arm-linux-gnueabihf" "Chroot Environment" {
                            containerInstance armhfScript "ARMHF Build"
                            containerInstance scopyApp "ARMHF Binary"
                        }
                    }
                }
            }
            deploymentNode "Azure DevOps Infrastructure" "Microsoft cloud CI/CD platform" "Azure DevOps" {
                deploymentNode "macOS Agent" "macOS" "macOS x86_64" {
                    deploymentNode "Homebrew Environment" "Package Manager" "macOS Native" {
                        containerInstance macosScript
                        containerInstance scopyApp "macOS Build"
                    }
                }
            }
        }

        distributionEnvironment = deploymentEnvironment "Artifact Distribution Environment" {
            deploymentNode "GitHub Releases" "Artifact Storage and Distribution" "GitHub" {
                containerInstance windowsInstaller
                containerInstance linuxFlatpak
                containerInstance linuxAppImage
                containerInstance arm64AppImage
                containerInstance armhfAppImage
                containerInstance ubuntuPackage
                containerInstance macosDMG
            }
        }
    }

    views {
        systemLandscape "SystemLandscape" "Overall Scopy deployment ecosystem" {
            include *
            exclude "relationship.tag==workflow"
        }

        container cicdPlatform "CICDPlatform" "CI/CD running platforms" {
            include *
        }

        container cicd "CICDContainers" "CI/CD workflow containers" {
            include *
            exclude cicdPlatform
            include githubActions azureDevOps
        }

        container deploymentTargets "DeploymentTargets" "Final deployment artifacts" {
            include *
        }

        deployment scopy buildEnvironment "BuildDeployment" "Multi-platform build infrastructure showing all compilation environments" {
            include *
        }

        deployment * distributionEnvironment "ArtifactDistribution" "Final artifact distribution showing all generated packages" {
            include *
        }

        dynamic cicd "BuildProcess" "Build process flow" {
            developer -> github "Push code"

            github -> githubActions "triggers GitHub Actions workflows"
            github -> azureDevOps "triggers Azure workflows"

            githubActions -> windowsWorkflow "Trigger Windows build"
            githubActions -> linuxFlatpakWorkflow "Trigger Linux builds"
            githubActions -> linuxAppImageWorkflow "Trigger Linux AppImage builds"
            githubActions -> ubuntuWorkflow "Trigger Ubuntu builds"
            githubActions -> arm64Workflow "Trigger ARM64 builds"
            githubActions -> armhfWorkflow "Trigger ARMHF builds"
            azureDevOps -> macosWorkflow "Trigger MacOS builds"

            windowsWorkflow -> mingwContainer "Uses"
            mingwContainer -> windowsScript "Execute Windows build"
            linuxFlatpakWorkflow -> flatpakContainer "Uses"
            flatpakContainer -> flatpakScript "Execute Flatpak build"
            linuxAppImageWorkflow -> x86Container "Uses"
            x86Container -> appimageScript "Execute AppImage build"
            ubuntuWorkflow -> ubuntuContainer "Uses"
            ubuntuContainer -> ubuntuScript "Execute Ubuntu build"
            arm64Workflow -> arm64Container "Uses"
            arm64Container -> arm64Script "Execute ARM64 builds"
            armhfWorkflow -> armhfContainer "Uses"
            armhfContainer -> armhfScript "Execute ARMHF builds"
            macosWorkflow -> macosScript "Execute macOS build"
            
            windowsScript -> windowsInstaller "Generate installer"
            appimageScript -> linuxAppImage "Generate AppImage"
            ubuntuScript -> ubuntuPackage "Generate Ubuntu build"
            flatpakScript -> linuxFlatpak "Generate Flatpak"
            arm64Script -> arm64AppImage "Generate ARM64 AppImage"
            armhfScript -> armhfAppImage "Generate ARMHF AppImage"
            macosScript -> macosDMG "Generate DMG"
            
            windowsInstaller -> artifactStore "Upload artifacts"
            linuxAppImage -> artifactStore "Upload artifacts"
            arm64AppImage -> artifactStore "Upload artifacts"
            armhfAppImage -> artifactStore "Upload artifacts"
            linuxFlatpak -> artifactStore "Upload artifacts"
            macosDMG -> artifactStore "Upload macOS artifacts"
            
        }

        styles {
            element "Software System" {
                background #1168bd
                color #ffffff
                shape RoundedBox
            }
            element "Container" {
                background #438dd5
                color #ffffff
                shape RoundedBox
            }
            element "Person" {
                background #08427b
                color #ffffff
                shape Person
            }
            element "External" {
                background #999999
                color #ffffff
            }

            relationship "Relationship" {
                dashed false
            }
            relationship "Async" {
                dashed true
            }
        }
    }
}