.. _linux_build_instructions:


Linux Build Instructions
==========================


Qt Setup Instructions
----------------------

Before building Scopy on a Linux system, ensure all necessary requirements are installed.
One key prerequisite is installing the Qt Framework (LTS version 5.15), which can be done using either the online or offline installer available on the official `Qt Downloads page <https://www.qt.io/download-dev>`_.

If you want to continue the development of the Scopy application, we recommend using the Qt Creator.
This can be installed with the official installer from `Qt Downloads page <https://www.qt.io/download-dev>`_, just make sure to also select the Qt Creator, alongside the Qt libraries, in the Qt Maintenance tool.

Another way to install the Qt Framework is to use the unofficial Qt CLI Installer `aqtinstall <https://github.com/miurahr/aqtinstall>`_.
Note that this method is not recommended if you want to continue developing the Scopy application or any other application that uses the Qt Framework. For a more secure method use the official installer.


To use aqtinstall, first clone the Scopy repository. With Python3 installed, execute the following command:

.. code-block:: bash

   $ scopy/ci/ubuntu/ubuntu_build_process.sh install_qt


Building Scopy
---------------

After the Qt installation is complete you can start following the next steps:

1. If not done yet, clone the Scopy repository:

   .. code-block:: bash

      $ cd ~
      $ git clone https://github.com/analogdevicesinc/scopy.git


2. Set up dependencies.

   - Run the following script with its respective arguments:

   .. code-block:: bash

      $ scopy/ci/ubuntu/ubuntu_build_process.sh install_packages build_deps


   This will install all packages needed to build the Scopy application. Note that this process may take a long time since Scopy requires many libraries to function properly.

   This script will require administrative rights to copy the libs and headers to known paths: `/usr/local/lib` and `/usr/local/include`.
   If you don't want that to happen, you can edit in the script the line that contains `USE_STAGING=OFF` to `USE_STAGING=ON`.
   But this will make the build process even more difficult and will not be covered in this tutorial.


3. Build the application.

   - From the root of the repository, create a build folder:

   .. code-block:: bash

      $ cd ~/scopy
      $ mkdir build
      $ cd build

   - From inside the build folder, execute CMake and point to the root of the repository:

   .. code-block:: bash

      $ cmake -DCMAKE_PREFIX_PATH=/opt/Qt/5.15.2/gcc_64 ../

   Depending on where you installed Qt, you may need to tell cmake where it was installed.
   If everything goes well, it should say `"Configuring done"` and `"Generating done"`.

   - After CMake completes, execute:

   .. code-block:: bash

      $ make

   This will compile the entire project. The process may take 15 minutes or more, depending on your CPU.
   To speed up the process, you can run make with ``-j`` argument followed by the number of threads that you want to use to build the app: ``make -j14``

   At the end of the build, an executable will be available, named simply: `scopy`.

   This can be run from the terminal and it should start the Scopy GUI.


Qt Creator project configuration
--------------------------------

In order to streamline the development of the Scopy application, we recommend using the `Qt Creator IDE <https://www.qt.io/product/development-tools>`_.

To start using Qt Creator for development, you have to first install all dependencies needed to build Scopy. To make sure you have this done right, follow the `Building Scopy`_ tutorial from above.

After that, open Qt Creator and click **Open Project**. Navigate to the location where you cloned Scopy and select the **CMakeLists.txt** file. Last step is to click **configure**, and Qt Creator should be able to build the application at the press of a button.
If there are errors at the building phase, try to first make a build manually from the terminal using the steps in `Building Scopy`_, and inside the Qt Creator, select the build folder and click **Configure**.

After completing these steps, Qt Creator will be ready to build the Scopy application.

Creating an Installer for x86_64 architecture
---------------------------------------------

If you want to create a Scopy installer for the Linux x86_64 architecture, you have two options: `AppImage` or `Flatpak`.


AppImage
""""""""
An `AppImage <https://docs.appimage.org/index.html>`_ is a self-contained file that includes all the necessary components for an application to run.
Once made executable and launched, the AppImage directly opens the application.

- To create a Scopy AppImage you will need the Scopy build dependencies to be already build and installed. If they are not installed, run the commad

.. code-block:: bash

   $ scopy/ci/ubuntu/ubuntu_build_process.sh install_packages build_deps

- After this step is completed, Scopy AppImage can be generated using the command:

.. code-block:: bash

   $ scopy/ci/x86_64/x86-64_appimage_process.sh install_packages run_workflow

This will download the packaging tools and use them to create an Scopy-x86_64.AppImage that can be easily distributed.


Flatpak
"""""""

A `Flatpak <https://docs.flatpak.org/en/latest>`_
- To use a Flatpak application, you first want to install the flatpak cli using the `instructions <https://flatpak.org/setup>`_.

- After that to use Flatpak for building an installer you will need to install the following packages and runtimes.

.. code-block:: bash

   $ sudo apt install -y build-essential subversion mm-common jq flatpak-builder
   $ sudo flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
   $ sudo flatpak install flathub org.kde.Platform//5.15-23.08 -y
   $ sudo flatpak install flathub org.kde.Sdk//5.15-23.08 -y

- After the installation is complete just run the script:

.. code-block:: bash

   $ scopy/ci/flatpak/flatpak_build_process.sh

This process will take some time because the flatpak build system will compile all dependencies needed for Scopy before packaging the application.


Creating an Installer for other architectures
---------------------------------------------

The Scopy application provides support for Linux on different architectures: `x86_64`, `armhf` and `arm64`.
The tutorial for building an installer for `x86_64` was presented above.

The Scopy installer for other architectures can be done locally on the host operating system, but it is out of the scope of this tutorial.
A shortcut that can be taken here is to download the Docker Image that is already configured to build and package the Scopy application.
All Docker Images are hosted on `Dockerhub <https://hub.docker.com/u/cristianbindea>`_, and can be freely downloaded. The recipe for each image can be found inside the `Scopy <https://github.com/analogdevicesinc/scopy/tree/main/ci>`_ repository.

Next, a temporary Docker volume is created to link the local environment with the Docker container.
The compilation process takes place inside the container using the pre-configured filesystem.
This Docker volume will include the previously cloned Scopy repository.


Installer for armhf
""""""""""""""""""""

1. The first step is to clone the Scopy repository, if not cloned yet

   .. code-block:: bash

      $ cd ~
      $ git clone https://github.com/analogdevicesinc/scopy.git

2. Download the Docker image

   .. code-block:: bash

      $ docker pull cristianbindea/scopy2-armhf-appimage:latest

3. Run the image, while creating a docker volume

   .. code-block:: bash

      $ docker run -it --mount type=bind,source=~/scopy,target=/home/runner/scopy cristianbindea/scopy2-armhf-appimage:latest

   Now this repository folder is shared with the Docker container.

4. To compile and package the application use

   .. code-block:: bash

      $ scopy/ci/arm/arm_build_process.sh arm32 run_workflow


Finally, after the development is done use this to clean the system:

.. code-block:: bash

   $ docker container ls -a # get the container id
   $ docker container rm -v (container id)
   $ docker image rm cristianbindea/scopy2-arm32-appimage:latest


Installer for arm64
""""""""""""""""""""

1. The first step is to clone the Scopy repository, if not cloned yet

   .. code-block:: bash

      $ cd ~
      $ git clone https://github.com/analogdevicesinc/scopy.git

2. Download the Docker image

   .. code-block:: bash

      $ docker pull cristianbindea/scopy2-arm64-appimage:latest

3. Run the image, while creating a docker volume

   .. code-block:: bash

      $ docker run -it --mount type=bind,source=~/scopy,target=/home/runner/scopy cristianbindea/scopy2-arm64-appimage:latest

   Now this repository folder is shared with the Docker container.

4. To compile and package the application use

   .. code-block:: bash

      $ scopy/ci/arm/arm_build_process.sh arm64 run_workflow


Finally, after the development is done use this to clean the system:

.. code-block:: bash

   $ docker container ls -a # get the container id
   $ docker container rm -v (container id)
   $ docker image rm cristianbindea/scopy2-arm64-appimage:latest
