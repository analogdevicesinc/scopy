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


Test 1 - PKG_MANAGER.DISCOVERY.LOAD
-----------------------------------

**UID:** PKG_MANAGER.DISCOVERY.LOAD

**Description:** Verify that all available packages are discovered and loaded correctly at startup.

**Preconditions:**
    - Scopy started with default package locations.

**Steps:**
    1. Start Scopy.
    2. Open the package manager UI or check logs for loaded packages.
        - **Expected result:**
            - All packages in the designated directories are listed and loaded without errors.
            - The following packages are expected to be loaded:

              * core
              * common
              * gui
              * gr-util
              * iioutil
              * iio-widgets
              * pluginbase
              * packages/generic-plugins
              * packages/ad936x
              * packages/dac
              * packages/m2k
              * packages/pluto
              * packages/pqm
              * packages/regmap
              * packages/swiot
              * packages/test
              * packages/test2
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

Test 2 - PKG_MANAGER.RESOURCE.ACCESS
--------------------------------------

**UID:** PKG_MANAGER.RESOURCE.ACCESS

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

Test 3 - PKG_MANAGER.CREATE
-----------------------------

**UID:** PKG_MANAGER.CREATE

**Description:** Test creating a new package (named "test_pkg") using the official package generator script (`package_generator.py`).

**Reference:** See :ref:`Package Generator <package_generator>`

**Preconditions:**
    - Python 3 and all prerequisites installed (see developer guide).
    - Scopy repository cloned and accessible.
    - In a terminal, working directory is `scopy/tools/packagegenerator`.

**Steps:**
    1. Prepare a `pkg.json` configuration file for the new package (see example in the developer guide).
    2. Run the following command to generate the package structure:

        .. code-block:: bash

            ./package_generator.py --all --config_file_path=path/to/pkg.json

    3. Verify that a new folder for "test_pkg" is created in the appropriate location with the expected structure and files (including a manifest file).
    4. (Optional) If using the package manager UI, check that "test_pkg" appears in the list of available packages (but not yet installed).
        - **Expected result:** The "test_pkg" package is created with the correct structure and metadata, and is available for installation.
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


Test 4 - PKG_MANAGER.INSTALL
------------------------------

**UID:** PKG_MANAGER.INSTALL

**Description:** Test installing the previously created package ("test_pkg") via the package manager and verifying it appears in the list and is usable.

**Preconditions:**
    - Test 3 (PKG_MANAGER.CREATE) was passed.
    - Scopy running with package manager UI accessible.

**Steps:**
    1. In the package manager UI, search for the package named "test_pkg" .
    2. Install the "test_pkg" package.
    3. Check that "test_pkg" appears in the package list and is enabled/usable.
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


Test 5 - PKG_MANAGER.UNINSTALL
--------------------------------

**UID:** PKG_MANAGER.UNINSTALL

**Description:** Test uninstalling the previously created and installed package ("test_pkg") via the package manager and verifying it is removed.

**Preconditions:**
    - Test 4 (PKG_MANAGER.INSTALL) was passed.
    - Scopy running with package manager UI accessible.

**Steps:**
    1. In the package manager UI, locate the "test_pkg" package.
    2. Uninstall the "test_pkg" package.
    3. Check that "test_pkg" is removed from the package list and its features are no longer available in Scopy.
        - **Expected result:** The "test_pkg" package is removed from the list and its features are not available.
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

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

**UID:** PKG_MANAGER.BACKWARD_COMPAT

**Description:** Verify that legacy plugins/packages are still recognized and function as expected, specifically by running a test from the m2k plugin.

**Preconditions:**
    - Legacy plugin/package (e.g., m2k) present in the package directory.
    - Scopy built with support for legacy plugins.

**Steps:**
    1. Start Scopy with the legacy m2k plugin/package installed. See :ref:`M2k Plugin <m2k>` for plugin details and usage.
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

**UID:** PKG_MANAGER.ERROR.HANDLING

**Description:** Ensure that errors (e.g., missing/corrupt package, bad resource) are handled gracefully.

**Preconditions:**
    - At least one broken or incomplete package present.

**Steps:**
    1. Start Scopy with a broken package in the directory.
        - **Expected result:** Error is reported to the user, but Scopy remains stable.
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

**UID:** PKG_MANAGER.UIUX

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

**UID:** PKG_MANAGER.MANUAL.DELETE

**Description:** Test deleting a package manually from the filesystem and observe Scopy’s behavior, using the AD936X package as the test subject.

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

**UID:** PKG_MANAGER.PLUGIN_INFO.ABOUT_PAGE

**Description:** Ensure that the About page for each package/plugin displays correct and updated information. Use the ADC generic plugin as a concrete example, which provides an `about.md` file.

**Preconditions:**
    - The ADC plugin is installed and enabled in Scopy.

**Steps:**
    1. Open the About page for the ADC plugin (e.g., via the plugin manager or plugin UI).
        - **Expected result:** The page displays the content from `plugins/adc/res/about.md`, including the correct name, version, and other metadata.
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

