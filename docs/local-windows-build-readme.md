This is a guide for installing the dependencies and setting up the environment for SCOPYv2 development. It is recommended to do these steps on a virtual machine since this has the potential to ruin the configuration of the system. The commands shown in this guide came from the [ci-for-scopy2](https://github.com/analogdevicesinc/scopy-mingw-build-deps/blob/ci-for-scopy2/docker/Dockerfile) dockerfile which can be visited and used as a reference. Most of the content of this guide is tailored fit to be executed using CMD or Windows Command Prompt which explains why there are some commands that are slightly different from the ones in the dockerfile. 

## Build prerequisites
- [Visual Studio Code](https://code.visualstudio.com/download)
- [LibIIO v0.25](https://github.com/analogdevicesinc/libiio/releases/download/v0.25/libiio-0.25.gb6028fd-setup.exe)
- [MSYS2](https://www.msys2.org/) (Use default installation settings. **Do not change the directory locations.** Uncheck Run MSYS2 now after installation.)

> If running Virtual Machines (e.g. VirtualBox, Hyper-V, VMWare, etc.) Open **Microsoft Store** and search for **OpenCL, OpenGL, and Vulkan Compatibility Pack**. This would install the packages needed in rendering the graphics of the software. Without this, Scopy would just crash.

## Build instructions
1. Create folder with the following directory *C:\msys64\home\docker*

2.  Make a **backup** of your user PATH variable. 

    > To make a backup, open run window using keyboard command **`WIN`**+**`R`** > Type **`SystemPropertiesAdvanced.exe`** and press the **`ENTER`** key > *System Properties* window will appear > Under the *Advanced* tab, click **`Environment Variables...`** > Under *User variables for (your user name)* find and click on *Path* and click **`Edit...`** > Then click **`Edit text...`** > copy and store the variable value in your preferred text editor > Save and close all opened windows.

3. Execute the following commands using Windows Command prompt *(CMD)* and **not MINGW64**: 
   
    ```sh
        cd C:/msys64/home/docker/
        set PATH=%PATH%;C:\msys64\bin;C:\msys64\mingw64\bin;C:\msys64\usr\bin
        setx HOME C:\msys64\home\docker
        setx CHERE_INVOKING yes
        setx MSYSTEM MINGW64 
        C:\msys64\usr\bin\bash.exe -lc " "
        C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Syyu"
        C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Sy msys2-keyring"
        C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Su"
        C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Syuu" & C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Syuu" & C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Scc "
        C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm --needed -Sy git"
        C:\msys64\usr\bin\bash.exe -lc "git clone https://github.com/analogdevicesinc/scopy-mingw-build-deps --branch ci-for-scopy2"
    ```
4. Check if **scopy-mingw-build-deps** directory is at *C:\msys64\home\\(your user name)\\*, if it is then move it to *C:\msys64\home\docker\\*

5. Continue to run commands
    ```sh
        C:\msys64\usr\bin\bash.exe -lc "cd /home/docker/scopy-mingw-build-deps && ls && echo Building for x86_64 && ./init_staging.sh x86_64 OFF"
        C:\msys64\usr\bin\bash.exe -lc "cd /home/docker/scopy-mingw-build-deps && source build.sh && install_tools && install_deps && recurse_submodules"
        C:\msys64\home\docker\scopy-mingw-build-deps\is.exe /VERYSILENT /SP- /SUPPRESSMSGBOXES /NORESTART /LOG=C:\msys64\home\docker\iss.log /DIR=C:\innosetup
        C:\msys64\usr\bin\bash.exe -lc "cd /home/docker/scopy-mingw-build-deps && source build.sh && build_deps"
    ```

6. Clone **scopy** using tag **dev** to the directory: *C:\msys64\home\docker*
    
    ```sh    
        C:\msys64\usr\bin\bash.exe -lc "git clone https://github.com/analogdevicesinc/scopy/ --branch dev"
    ```

7. Check if **scopy** directory is at *C:\msys64\home\\(your user name)*, if it is then move it to *C:\msys64\home\docker\\*

8. Install GDB for build debugging
    ```sh
        C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -S mingw-w64-x86_64-gdb"
    ```

9. In VS Code, install [**C/C++ Extension Pack**](vscode:extension/ms-vscode.cpptools-extension-pack)

10. Open Scopy folder in VS Code

    > When opening Scopy folder for the first time, a popup may appear to ask to trust the authors of the files in this folder. Simply click on **`Yes, I trust the authors`**

11. In VS Code, go to the toolbar on your left and locate CMake tool. On the **PROJECT OUTLINE** dropdown, click on the icon for *Configure All Projects*. This will instruct CMake to build the scripts necessary in building the source code.

12. Under the **PROJECT STATUS** dropdown in CMake tool, click on the icon for *Build* to build the project.