.. _windows_build_instructions:

Windows Build Instructions
==========================


Building Scopy
-----------------------

In order to emulate a UNIX-like environment for building Scopy on Windows, we use MSYS2.

To install MSYS2, follow these instructions: `MSYS2 Installation <https://www.msys2.org/#installation>`_.
We suggest installing the MSYS2 in root of the **C:\\** partition. In this tutorial the MSYS2 install path is **C:\\msys64**, if you choose a different install directory, make sure to also update the paths.

.. tip::
    Open the **MINGW64** shell of MSYS2. Other terminals will not work!

1. Clone the Scopy repository:

   .. code-block:: bash

      $ cd ~
      $ git clone https://github.com/analogdevicesinc/scopy.git

2. Set up dependencies:

   - Execute the script:

   .. code-block:: bash

      $ scopy/ci/windows/windows_build_process.sh

   This will install all packages needed to build the Scopy application. Note that this process may take a long time since Scopy requires many libraries to function properly.

3. Build the application:

   - From the root of the repository, create a build folder:

   .. code-block:: bash

      $ cd ~/scopy
      $ mkdir build
      $ cd build

   - From inside the build folder, execute CMake and point to the root of the repository:

   .. code-block:: bash

      $ cmake -GNinja ../

   If everything goes well, it should say "Configuring done" and "Generating done."
   - After CMake completes, execute:

   .. code-block:: bash

      $ ninja


This will compile the entire project. The process may take 15 minutes or more, depending on
your CPU. At the end of the build, two executables will be available: `scopy.exe` and `scopy-console.exe`.
Both start the application, but the latter runs Scopy in debug mode.


Running Scopy
-----------------------

To run Scopy, it is required to first set a few environment variables. To do that, click on the Start menu and search for "environment".
The option "Edit environment variables for your account" should appear; Click on it.

You will need to create two environment variables: **PATH** and **SCOPY_PYTHONPATH**.

We assume that MSYS2 installation path is **C:\\msys64**, if have chosen a different install directory, make sure to also update the paths.

1. Add the **PATH** variable:

   - Click on **"New..."**
   - In **"Variable name:"**, write ``PATH``
   - In **"Variable value:"**, write: ``%PATH%;C:\msys64\mingw64\bin``
   - Click **OK**.

2. Add the **SCOPY_PYTHONPATH** variable:

   - For this variable you would need to check that the python version installed in MINGW64. To do this run the command:

   .. code-block:: bash

      $ python --version

   - Click on **"New..."**
   - In **"Variable name:"**, write ``SCOPY_PYTHONPATH``
   - In **"Variable value:"**, write the line below. Also update the python version if needed.

   .. code-block::

         C:\msys64\mingw64\lib\python3.11;C:\msys64\mingw64\lib\python3.11\plat-win;C:\msys64\mingw64\lib\python3.11\lib-dynload;C:\msys64\mingw64\lib\python3.11\site-packages

   - Click OK.

Once these steps are complete, the Scopy you built should run. Using File Explorer, navigate to:

.. code-block::

   <MSYS2 Installation Path>/mingw64/home/scopy/build

- Double-click on **scopy.exe**.


Windows Qt Setup Instructions
-----------------------------

To avoid using the MSYS terminal for rebuilding the app, you can use Qt Creator instead. Follow the steps below to set up and use Qt Creator:

1. Install Qt Creator:

   - Download the Qt Creator installer (offline or online) from the official `Qt Downloads page <https://www.qt.io/download-dev>`_.
   - During the installation process, select only the **Qt Creator** option. This ensures that only the Qt Creator IDE is installed. The necessary Qt libraries will be sourced from the MSYS2 folder.
   - Once the installation is complete, open Qt Creator.

2. Initial Configuration for Scopy Application:

   - Follow the steps outlined in the `Building Scopy`_ section to set up the environment.

3. Configure the Project in Qt Creator:

   Considering that you already done a build from the MINGW64 terminal. If not, follow the steps from `Building Scopy`_

   - Open Qt Creator and click **Open Project**.
   - Navigate to the MSYS2 installation directory and locate the Scopy project files. The typical path structure is:

   .. code-block::

      <MSYS2 Installation Path>/mingw64/home/scopy/build

   - Select the build folder and click **Configure**.

After completing these steps, Qt Creator will be ready to build the Scopy application, streamlining the process compared to using the MSYS terminal.


Creating an Installer
----------------------------

To create an installer, the first step is to install the `Inno Setup Compiler <https://files.jrsoftware.org/is/6/innosetup-6.2.2.exe>`_.

After that, there is only one more step:

- Run the command:

   .. code-block:: bash

      $ scopy/ci/windows/build_and_create_installer.sh run_workflow

This script will build the Scopy application and package it into a `Scopy-installer.exe` file. This file contains all the necessary components for Scopy to run properly, making it easy to redistribute.


Building Scopy using the Docker Image
---------------------------------------

Another way to build the Scopy application is by using pre-configured Docker images. A Docker image
is essentially a snapshot of an operating system that includes all the libraries and dependencies required to build the application.

This tutorial has only two requirements:

- `Git <https://git-scm.com/downloads/win>`_: Used to download the source code from GitHub.
- `Docker Desktop <https://docs.docker.com/desktop/setup/install/windows-install/>`_: Used to download and run the Docker image.

To simplify the process, you can download a pre-configured Docker image specifically set
up to build and package the Scopy application. These Docker images are hosted on `Docker Hub <https://hub.docker.com/u/cristianbindea>`_
and are free to download. Additionally, the recipe for each image is available in the `Scopy repository <https://github.com/analogdevicesinc/scopy/tree/main/ci>`_.

After you have installed Docker Desktop, the next steps will be run inside a powershell terminal.

1. Clone the Scopy repository

   .. code-block:: powershell

      > git clone https://github.com/analogdevicesinc/scopy.git

2. Download the Docker Image

   .. code-block:: powershell

      > docker pull cristianbindea/scopy2-mingw64:latest

3. Start the Docker Image

   Once the image is downloaded, you will create a temporary Docker volume to
   connect your local environment with the Docker container.

   The compilation process takes place inside the container using its pre-configured
   filesystem. The Docker volume will include the previously cloned Scopy repository to ensure everything is in sync.

   Make sure to update the source path to actual repository location.

   .. code-block:: powershell

      > docker run -it --mount type=bind,source=C:\Users\analog\scopy,target=/home/runner/scopy cristianbindea/scopy2-mingw64:latest

   Now this repository folder it is shared with the Docker container.

4. Run the build script

   - From inside the Docker Image, run the build script:

   .. code-block:: powershell

      > C:\msys64\usr\bin\bash.exe -c '/home/docker/scopy/ci/windows/build_and_create_installer.sh  run_workflow'

   This script will build and package the application.

5. Locate the binary file

   After the script has finished running, you can install, or open the Scopy application.
   Enter the repository location using File Explorer and you will find the Scopy installer `Scopy-installer.exe`.
