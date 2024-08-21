This is a guide for installing the dependencies and setting up the environment for Scopy 2.0. The commands shown in this guide came from the [ci-for-scopy2](https://github.com/analogdevicesinc/scopy-mingw-build-deps/blob/ci-for-scopy2/docker/Dockerfile) dockerfile which can be visited and used as a reference.

## Build prerequisites
- IDE
    - [QTCreator](https://doc.qt.io/qtcreator/)
    - [Visual Studio Code](https://code.visualstudio.com/download)
- [MSYS2](https://www.msys2.org/)

## Configuring PATH

1.  Make a **backup** of your user PATH variable. 

    > To make a backup, open run window using keyboard command **`WIN`**+**`R`** > Type **`SystemPropertiesAdvanced.exe`** and press the **`ENTER`** key > *System Properties* window will appear > Under the *Advanced* tab, click **`Environment Variables...`** > Under *User variables for (your user name)* find and click on *Path* and click **`Edit...`** > Then click **`Edit text...`** > copy and store the variable value in your preferred text editor > Save and close all opened windows.

2. Append MSYS to PATH
    ```sh
        set PATH=%PATH%;C:\msys64\bin;C:\msys64\mingw64\bin;C:\msys64\usr\bin
    ```

## Setting up dependencies

1. Launch bash using the following command using Windows Command prompt

    ```sh
        C:\msys64\usr\bin\bash.exe
    ```

2. Execute the following commands in the bash terminal
   
    ```bash
        pacman --noconfirm -Syyuu
        pacman --noconfirm --needed -Sy git
        git clone https://github.com/analogdevicesinc/scopy-mingw-build-deps --branch ci-for-scopy2
        cd /home/docker/scopy-mingw-build-deps/build.sh install_tools install_deps build_deps
    ```

3. Clone **Scopy** using tag **dev**
    
    ```bash
        git clone https://github.com/analogdevicesinc/scopy/ --branch dev
    ```

4. Install GDB for build debugging
    ```bash
        pacman --noconfirm -S mingw-w64-x86_64-gdb
    ```

## Building in Visual Studio Code

1. In VS Code, install [**C/C++ Extension Pack**](vscode:extension/ms-vscode.cpptools-extension-pack)

2. Open Scopy folder in VS Code

    > When opening Scopy folder for the first time, a popup may appear to ask to trust the authors of the files in this folder. Simply click on **`Yes, I trust the authors`**

3. In VS Code, go to the toolbar on your left and locate CMake tool. On the **`PROJECT OUTLINE`** dropdown, click on the icon for *Configure All Projects*. This will instruct CMake to build the scripts necessary in building the source code.

4. Under the **`PROJECT STATUS`** dropdown in CMake tool, click on the icon for *Build* to build the project.