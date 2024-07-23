# Scopy MinGW Recipe

Build Scopy for Windows using MinGW64

Scopy is a software oscilloscope and signal analysis toolset. [The official repository](https://github.com/analogdevicesinc/scopy) provides releases for Windows, Linux, macOS and Android.


## Building the Docker image
To build the Docker image just execute the command

``` docker build --progress=plain --tag <image_name> --isolation=hyperv --memory=16GB --file docker/Dockerfile .```

The Dockerfile is available in the docker folder.

[MSYS2](https://www.msys2.org/) will be installed inside the image along with all dependencies that are required in order to build and package the Scopy app.


## Setting up the environment

### Build prerequisites
- An IDE:
    - [QTCreator](https://doc.qt.io/qtcreator/)
    - [Visual Studio Code](https://code.visualstudio.com/download)

 - For development the [MSYS2](https://www.msys2.org/) shell is needed.

### Setup dependencies

1. Launch the mingw64 by executing the binary file from the location where the MSYS2 was installed

2. Execute the following script in the mingw64 terminal

    ```bash
        windows_build_process.sh
    ```

- Optionally install GDB for build debugging
    ```bash
        pacman --noconfirm -S mingw-w64-x86_64-gdb
    ```

### Building inside MinGW64 shell

1. Enter the root directory of the repository
2. Create a build directory ```mkdir build```
3. Enter the build directory ```cd build```
4. Execute ```cmake ../```
5. Execute ``` make```

Inside the build folder a binary file named Scopy.exe will be generated. This is the starting file of the application.

### Building in Visual Studio Code

1. In VS Code, install [**C/C++ Extension Pack**](vscode:extension/ms-vscode.cpptools-extension-pack)

2. Open Scopy folder in VS Code

    > When opening the Scopy folder for the first time, a popup may appear to ask you to trust the authors of the files in this folder. Simply click on **`Yes, I trust the authors`**

3. In VS Code, go to the toolbar on your left and locate the CMake tool. On the **`PROJECT OUTLINE`** dropdown, click on the icon for *Configure All Projects*. This will instruct CMake to generate the files necessary for building the source code.

4. Under the **`PROJECT STATUS`** dropdown in the CMake tool, click on the icon for *Build* to build the project.

## Generating the Scopy installer
The installer is created using [Inno Setup](https://jrsoftware.org/isinfo.php). This tool is already installed in the Docker Image, but to install Inno Setup locally use this [installer](https://files.jrsoftware.org/is/6/innosetup-6.2.2.exe).

First follow the **Setup dependencies** part.

In order to build Scopy and create an installer locally use the **build_and_create_installer.sh** bash script executed from the mingw shell.
