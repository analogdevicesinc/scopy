.. _license_and_about_page_tests:

License and About Page
========================================

.. note::
    .. list-table:: 
       :widths: 50 30 30 50 50
       :header-rows: 1

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

Test 1: Check license pop-up
-----------------------------

**UID:** TST.LICENSE.POPUP

**Preconditions:**
    - OS: Any

**Description:** This test verifies if the license pop-up appears at the start of the application.

**Steps:**
    1. Start the Scopy application.
    2. Click on **Preferences**
    3. Scroll down, click on **Reset** and after that on **Restart**
        - **Expected result:** The application should restart and the first thing
          that appears on the screen is a pop-up regarding the license used in Scopy.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click on the highlighted text **"GPLv3"**
        - **Expected result:** A web browser window will open containing the GPLv3
          license used by Scopy.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Click on **Continue**
        - **Expected result:** The application will display the home screen with
          the text "Welcome to Scopy!".
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

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).



Test 2: Check links from About Page
-------------------------------------

**UID:** TST.ABOUT.PAGE

**Preconditions:**
    - OS: Any

**Description:** This test verifies if all the links from **About Page** work correctly.

**Steps:**
    1. Start the Scopy application.
    2. Click on the **About** button from the bottom left side of the application.
        - **Expected result:** The application will display a screen containing
          textual information about the current version.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click on the underlined word **here** from the line **Build info: here**
        - **Expected result:** The displayed text will change, and it now shows
          more detailed information about the building process.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click on the **Home** buttom from the top right side of the application.
        - **Expected result:** The application will display the initial screen
          of the About page.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Click on the highlighted text **Scopy open source project**
        - **Expected result:** The application will display more information about
          the Scopy project.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Click on the **Home** buttom from the top right side of the application.
    7. Click on the highlighted text **other open source software**
        - **Expected result:** The application will display a list of open source
          libraries included or linked to Scopy.
        - **Actual result:**

..
  Actual test result goes here.
..

    8. Click on the **Home** buttom from the top right side of the application.
    9. Click on the highlighted text **Latest Release**
        - **Expected result:** A web browser window will open containing the latest
          release of Scopy.
        - **Actual result:**

..
  Actual test result goes here.
..

    10. Click on the **Home** buttom from the top right side of the application.
    11. Click on the highlighted text **Documentation**
         - **Expected result:** A web browser window will open containing the
           documentation for Scopy.
         - **Actual result:**

..
  Actual test result goes here.
..

    12. Click on the **Home** buttom from the top right side of the application.
    13. Click on the highlighted text **Support Forum**
         - **Expected result:** A web browser window will open containing a forum
           named `ADI EngineerZone`.
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

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
