# Ubuntu CI Directory

## Overview

This directory contains scripts for building Scopy on Ubuntu Linux. These builds are primarily for development purposes. Using the scripts from this folder the development environment for Linux can be easily set-up.

### Scripts

#### `ubuntu_build_process.sh`- Main build orchestration script

#### `create_docker_image.sh` - Creates Docker image for Ubuntu builds

#### `docker_ubuntu/Dockerfile`- Defines the Ubuntu build environment

## Development Setup

The build script steps:

1. Updates package repositories
2. Installs build dependencies, including the Qt framework
3. Builds and installs the dependencies in correct order
4. Builds Scopy

To run the environment setup process run:

```bash
   ./ubuntu_build_process.sh configure_system
```

This will install and build all required dependencies, but it will not build Scopy itself.
By default, the script requires administrator permissions to install the dependencies. However, if multiple versions of the same library are needed, the required libraries can instead be installed locally. To enable this local build, edit the *ubuntu_build_process.sh* script and change the `USE_STAGING` option from `OFF` to `ON`.

To built the Scopy application:

```bash
   ./ubuntu_build_process.sh build_scopy
```

## CI Integration

- **GitHub Actions**: `.github\workflows\ubuntubuild.yml`

## Notes

- This is primarily for development and testing, not end-user distribution
- Requires Ubuntu-specific package versions
- May need adjustments for different Linux distributions
