.. _package_manager_tests:

Package Manager - Test Suite
======================================

.. note::
    .. list-table:: 

       * - Tester
         - Test Date
         - Scopy version
         - Plugin version (N/A if not applicable)
         - Comments
       * - 
         - 
         - 
         - 
         - 


Test 1 - Package Discovery Load
-----------------------------------

**UID:** TST.PKG_MANAGER.DISCOVERY.LOAD

**RBP:** P0

**Description:** Verify that all available packages are discovered and loaded correctly at startup.

**Preconditions:**
    - Scopy started with default package locations.

**Steps:**
    1. Start Scopy.
    2. Open the package manager UI or check logs for loaded packages.
        - **Expected result:**
            - All packages in the designated directories are listed and loaded without errors.
            - The following packages are expected to be loaded:

              * ad936x, 
              * apollo ad9084, 
              * generic plugins, 
              * adalm2000, 
              * power quality monitor, 
              * ad-swiot1l-sl

        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..

Test 2 - Package Resource Access
--------------------------------------

**UID:** TST.PKG_MANAGER.RESOURCE.ACCESS

**RBP:** P1

**Description:** Ensure that resources (XML, CSV, etc.) within packages are accessible and used by the application.

**Preconditions:**
    - At least one package with resources installed.
    - Registermap tests were passed and the tool is working properly.

**Steps:**
    1. Connect a physical Pluto device or start Pluto in emulation mode.
    2. Open the Register Map tool in Scopy.
    3. Select the connected Pluto device.
    4. Check if the register table loads and displays correctly.
        - **Expected result:** The register table is loaded from the XML resource and displayed without errors.
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..

Test 3 - Package Create
-----------------------------

**UID:** TST.PKG_MANAGER.CREATE

**RBP:** P2

**Description:** Test creating a new package (named "test_pkg") using the official package generator script (`package_generator.py`).

**Reference:** See :ref:`Package Generator <package_generator>`

**Preconditions:**
    - Python 3 and all prerequisites installed (see developer guide).
    - Scopy repository cloned and accessible.
    - In a terminal, working directory is `scopy/tools/packagegenerator`.

**Steps:**
    1. Prepare a `pkg.json` configuration file for the new package (see example in the developer guide).
    2. Run the following command to generate the package template:

        .. code-block:: bash

            ./package_generator.py --all --config_file_path=path/to/pkg.json

    3. Verify that a new folder for "test_pkg" is created as a development template in `scopy/packages/` with the expected structure and files (including a manifest file).
        - Folder hierarchy when using the `--all` flag should look like:

          .. code-block:: none

             ├── CMakeLists.txt
             ├── include
             │   └── newpackage
             ├── manifest.json.cmakein
             ├── plugins
             │   └── newplugin
             ├── resources
             │   ├── translations
             │   └── translations.qrc
             └── style
                 ├── json
                 └── qss
               ├── generic
               └── properties

    4. If a build is performed, the new package should be present in the build folder as well.
        - **Expected result:** The "test_pkg" package template is created in `scopy/packages/` with the correct structure and metadata, and after build, it appears in the build folder.
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 4 - Package Install
------------------------------

**UID:** TST.PKG_MANAGER.INSTALL

**RBP:** P2

**Description:** Test installing the previously created package ("test_pkg") via the package manager and verifying it appears in the list and is usable.

**Preconditions:**
    - Test 3 (TST.PKG_MANAGER.CREATE) was passed.
    - Scopy running with package manager UI accessible.

**Steps:**
    1. Create a zip file of the "test_pkg" package or ensure it is in the correct format for installation.
    2. In the package manager UI, search for the zipped package named "test_pkg".
    3. Install the "test_pkg" package.
    4. Check that "test_pkg" appears in the package list and is enabled/usable.
        - **Expected result:** The "test_pkg" package appears in the list and its features are available in Scopy.
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 5 - Package Uninstall
--------------------------------

**UID:** TST.PKG_MANAGER.UNINSTALL

**RBP:** P2

**Description:** Test uninstalling the previously created and installed package ("test_pkg") via the package manager and verifying it is removed.

**Preconditions:**
    - Test 4 (TST.PKG_MANAGER.INSTALL) was passed.
    - Scopy running with package manager UI accessible.

**Steps:**
    1. In the package manager UI, locate the "test_pkg" package.
    2. Uninstall the "test_pkg" package.
    3. A button should appear for a Restart.
    4. Press the Restart button.
    5. After Scopy restarts, check that "test_pkg" is removed from the package list and its features are no longer available in Scopy.
        - **Expected result:** The "test_pkg" package is removed from the list and its features are not available.
        - **Actual result:**


..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..

Test 6 - Backward compatibility
----------------------------------

**UID:** TST.PKG_MANAGER.BACKWARD_COMPAT

**RBP:** P3

**Description:** Verify that legacy plugins/packages are still recognized and function as expected, specifically by running a test from the m2k plugin.

**Preconditions:**
    - Legacy plugin/package (e.g., m2k) present in the package directory.
    - Scopy built with support for legacy plugins.

**Steps:**
    1. Start Scopy with the legacy m2k plugin/package installed. See :ref:`M2k Plugin <m2k_index>` for plugin details and usage.
    2. Open the m2k plugin UI or access its features.
    3. Run a basic test or operation provided by the m2k plugin (e.g., connect to an m2k device and perform a simple acquisition).
        - **Expected result:** The m2k plugin loads, its UI/features are accessible, and the test operation completes successfully without errors.
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..

Test 7 - Error handling
---------------------------------

**UID:** TST.PKG_MANAGER.ERROR.HANDLING

**RBP:** P3

**Description:** Ensure that errors (e.g., missing/corrupt package, bad resource) are handled gracefully.

**Preconditions:**
    - Scopy running with package manager UI accessible.

**Steps:**
    1. Create an archive (zip) of a package but omit the manifest file (e.g., remove `manifest.json` before archiving).
    2. Attempt to install the corrupted package using the package manager UI.
    3. Observe the result.
        - **Expected result:** An error is reported and Scopy remains stable without crashing or freezing.
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..

Test 8 - UI/UX
----------------------

**UID:** TST.PKG_MANAGER.UIUX

**RBP:** P3

**Description:** Check that the package manager UI is clear, responsive, and provides necessary feedback.

**Preconditions:**
    - Scopy running with package manager UI enabled.

**Steps:**
    1. Open the package manager UI.
    2. Perform install/uninstall and observe feedback.
        - **Expected result:** UI updates in real time, shows status, and is user-friendly.
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 9 - Manual package deletion
-----------------------------------

**UID:** TST.PKG_MANAGER.MANUAL.DELETE

**RBP:** P3

**Description:** Test deleting a package manually from the filesystem and observe Scopy's behavior, using the AD936X package as the test subject.

**Preconditions:**
    - The AD936X package is installed.

**Steps:**
    1. Close Scopy.
    2. Delete the `ad936x` package folder from the package directory.
    3. Restart Scopy.
        - **Expected result:** The AD936X package is no longer listed or loaded, and no errors occur.
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..



Test 10 - Plugin info About page
-----------------------------------------

**UID:** TST.PKG_MANAGER.PLUGIN_INFO.ABOUT_PAGE

**RBP:** P3

**Description:** Ensure that the "Plugins Info" tab in the About section lists all plugins that are currently loaded (i.e., have the "loaded" label).

**Preconditions:**
    - Multiple plugins installed and enabled in Scopy.

**Steps:**
    1. Open the About section in Scopy.
    2. Navigate to the "Plugins Info" tab.
    3. Verify that all plugins which are loaded are listed and have the "loaded" label.
        - **Expected result:** The "Plugins Info" tab displays all loaded plugins with the correct label, reflecting the actual state of the application.
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..

