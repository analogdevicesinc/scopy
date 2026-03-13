# Windows CI Directory

## Folder overview

This directory contains scripts and resources for building Scopy on Windows using the MinGW toolchain.
The build process uses MSYS2/MinGW environment to build and install the dependencies and creates Windows installers using Inno Setup.

More detailed information can be found here: [Windows Build Instructions](https://analogdevicesinc.github.io/scopy/user_guide/build_instructions/windowsBuild.html#windows-build-instructions).

### Scripts

#### `mingw_toolchain.sh` - Sets up MinGW toolchain environment variables

#### `windows_build_process.sh` - Main Windows build orchestration script

#### `build_and_create_installer.sh` - Builds Scopy and creates Windows installer

### Supporting Files

#### `scopy-64.iss.cmakein` - Inno Setup installer template  (will be processed by CMake to generate final .iss file)

#### `mingw_dll_deps` -  Lists all DLLs needed for the installer

#### `sigrokdecode-windows-fix.patch` - Windows-specific patch for libsigrokdecode

### Docker Support

#### `docker/Dockerfile` - Creates Windows build environment

### Drivers Directory

#### `drivers/` - Contains Windows driver files for hardware support

## Build Process

### Building Scopy using the Docker Image

`Prerequisites:` [Docker](https://docs.docker.com/desktop/setup/install/windows-install/)

1. **Pull the Docker image**

   ```bash
   docker pull cristianbindea/scopy2-mingw64:latest
   ```

2. **Start the Docker Image**

   ```bash
   docker run -it cristianbindea/scopy2-mingw64:latest
   ```

3. **Enter the MSYS env**

   ```bash
      C:\msys64\usr\bin\bash.exe
   ```

4. **Clone and build the application**

   ```bash
      git clone https://github.com/analogdevicesinc/scopy
      cd scopy
      mkdir build && cd build
      cmake ../
      make -j$(nproc)
   ```

This will compile the entire project. The process may take 15 minutes or more, depending on your CPU. More information about how to create a shared folder between the Docker container and the local machine can be found here [Building Scopy using the Docker Image](https://analogdevicesinc.github.io/scopy/user_guide/build_instructions/windowsBuild.html#building-scopy-using-the-docker-image).

### Building the Docker Image

`Prerequisites:` [Docker](https://docs.docker.com/desktop/setup/install/windows-install/)

To build the Docker image just execute the command:

   ```bash
      docker build --tag scopy-windows --isolation=hyperv --memory=16GB --file docker/Dockerfile .
   ```

This will create the Docker Image and will install on it every dependency needed to build Scopy and to create an installer.

### Building locally from sources

`Prerequisites:`

- [MSYS2](https://www.msys2.org/) with MinGW64
- [Inno Setup](https://jrsoftware.org/isdl.php) (only if you want to create and installer)
- ~20GB free disk space

In order to emulate a UNIX-like environment for building Scopy on Windows, we use MSYS2.

To install MSYS2, follow these instructions: MSYS2 Installation. We suggest installing the MSYS2 in root of the C:\ partition. In this tutorial the MSYS2 install path is C:\msys64, if you choose a different install directory, make sure to also update the paths.

1. **Launch MinGW64 shell** from MSYS2 installation

2. **Install Git and clone Scopy**

   ```bash
      pacman -S git
      git clone https://github.com/analogdevicesinc/scopy
   ```

3. **Setup dependencies**

   ```bash
      ./windows_build_process.sh
   ```

This will configure the system for building Scopy.

4. **Build Scopy**

You can manually build the application using:

   ```bash
      mkdir build && cd build
      cmake ../
      make -j$(nproc)
   ```

Or you can run the build script. This will do a clean build each time is called.

   ```bash
      ./build_and_create_installer.sh build_scopy
   ```

Inside the build folder two executable binary files named Scopy.exe and Scopy-console.exe will be generated. This is the starting file of the application. More information about how to add the app to the PATH and open it from the file explorer, not just from the MINGW64 command line, can be found here [Running Scopy](https://analogdevicesinc.github.io/scopy/user_guide/build_instructions/windowsBuild.html#running-scopy).

### Create installer

In order to create an installer make sure you have Inno Setup installed at the default location. Or you can change the PATH variable from *build_and_create_installer.sh / create_installer()*.

   ```bash
      ./build_and_create_installer.sh build_scopy build_iio-emu deploy_app extract_debug_symbols create_installer
   ```

## Development Setup

In order to have a complete Development setup you should follow the first 3 steps from [Build from source](###Building-locally-from-sources).

A shortcut in the setup can be taken by first building the app from the command line using the step 4 from [Build from source](###Building-locally-from-sources). This will configure the build and will compile the application for the first time, later you can just import this configuration intro VS Code or Qt Creator.

### Visual Studio Code

1. Install C/C++ Extension Pack
2. Open Scopy folder in VS Code
3. Configure CMake project (CMake tool → Configure All Projects)
4. Build project (CMake tool → Build)

### Qt Creator

1. Open project CMakeLists.txt
2. Configure with MinGW64 kit
3. Build and run

Here you can find a more detailed tutorial on [Qt Setup](https://analogdevicesinc.github.io/scopy/user_guide/build_instructions/windowsBuild.html#windows-qt-setup-instructions).

### Debugging

Install GDB for debugging:

```bash
   pacman --noconfirm -S mingw-w64-x86_64-gdb
```

## Output

- **Scopy.exe**: Main executable in build directory
- **Scopy-console.exe**: Main executable in build directory, but with console logs
- **Scopy-setup.exe**: Inno Setup installer
- All required DLLs and runtime files

## CI Integration

- **GitHub Actions**: `.github/workflows/mingwbuild.yml`

## Troubleshooting

Common issues:

1. **"command not found"**: Ensure running in MinGW64 shell, not CMD
2. **Missing DLLs**: Check `mingw_dll_deps` list
3. **CMake errors**: Verify all dependencies built successfully
4. **Installer fails**: Ensure Inno Setup is installed
5. **Out of memory**: Docker builds need 16GB RAM
