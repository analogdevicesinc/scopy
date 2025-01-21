.. _macos_build_instructions:

MacOS Build Instructions
==========================

.. caution::

    The process of building and correctly linking the application and it's dependencies is complex and very error prone.
    To simplify the process we created a series of bash scripts that take care of this, the downside is that compiling and packaging takes a lot of time.
    Currently without modifying the source code you can only build the application for intel x86_64 processors.
    We are working to optimize this build procedure.


Building Scopy
-----------------------

1. Clone the Scopy repository:

   .. code-block:: zsh

      % cd ~
      % git clone https://github.com/analogdevicesinc/scopy.git


2. Set up dependencies.

   - Run the following script:

   .. code-block:: zsh

      % scopy/ci/macOS/install_macos_deps.sh

   This will install packages using brew, so you will have to make sure that you have brew installed on the machine. Here are the `install instructions <https://docs.brew.sh/Installation>`_. The rest of the dependencies that can't be found on brew will be built from the source files.

3. Build the application

   - Run the following script:

   .. code-block:: zsh

      % scopy/ci/macOS/build_azure_macos.sh

   This will use the packages from `Step 2` in order to compile the Scopy application.

4. Package the application

   - Run the following script:

   .. code-block:: zsh

      % scopy/ci/macOS/package_darwin.sh

   To run the application, the final step is linking the dependencies to the Scopy binary, enabling the operating system to locate them at runtime.

   This process is handled by a script that manages both the linking and packaging. Once the script completes, inside the build folder, it generates a file named **Scopy.app**, which can be opened either by running **"open Scopy.app"** in the terminal or by double-clicking it in the file explorer.