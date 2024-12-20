.. _power_supply_tests:

Power Supply - Test Cases
=========================

Initial Setup
-------------

In order to proceed through the test case, first of all delete the Scopy \*.ini file (saves previous settings made in Scopy tool) from the following path on Windows: ``C:\Users\your_username\AppData\Roaming\ADI``.

Open the Power Supply instrument. The interface should look like the picture below:

Press multiple times on the “Enable” buttons to check if the instrument works.

Test Title
----------

A. Independent Controls
------------------------

Description
-----------

Checking positive voltage output
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Set Tracking ratio control to Independent Controls.
    - The interface should look like in the “Step Resources” picture (left side).

2. Connect the power supply and voltmeter with the following pins: V+ power supply pin (red) to Scope Ch 1 (orange).

Setting values
~~~~~~~~~~~~~~

3. Set the value to 3.3V and click enable.
    - The interface should look like in the “Step Resources” picture (left side).

4. Monitor the power supply output with voltmeter.
    - The voltmeter should read values between 3.25V and 3.35V. Just like shown on the left.

Changing set values
~~~~~~~~~~~~~~~~~~~

5. Change the power supply output value to 1.8V.
    - The voltmeter should read values between 1.75V and 1.85V.

6. Change the power supply output value to 2.5V.
    - The voltmeter should read values between 2.45V and 2.55V.

7. Change the power supply output value to 5V.
    - The voltmeter should read values between 4.95V and 5.05V.

Checking Increment/Decrement Value; ±1V
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

8. Set the knob to ±1V interval. No orange dot on the center.
    - The interface should look like in the “Step Resources picture (left side).

9. Set value to 3V. Then use +/- sign to change value with ±1V interval.
    - The value should change accordingly. Set Value ± 1V = the new value.

Checking Increment/Decrement Value; ±100mV
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

10. Set the knob to ±100mV interval. Orange dot seen on the center.
    - The interface should look like in the “Step Resources picture (left side).

11. Set value to 300mV. Then use +/- sign to change value with ±100mV interval.
    - The value should change accordingly. Set Value ± 100mV = the new value.

Checking Negative Voltage Output
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

12. Change pin connections to the following: V- power supply pin (white) to Scope Ch 1 (orange).

13. Repeat Steps 3 to 11. Set the values mentioned to negative in checking negative output.

B. Tracking
-----------

Description
-----------

Checking output when in Tracking mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Set Tracking ratio control to Tracking.
    - The interface should look like in the “Step Resources” picture (left side).

2. Connect the following pins: V+ power supply pin (red) to Scope Ch 1 (orange); V- power supply pin (white) to Scope Ch 2 (blue).

Setting tracking ratio
~~~~~~~~~~~~~~~~~~~~~~

3. Set value of positive output to 5V. Set tracking ratio to 50%.
    - The negative output value should be set automatically following the equation: V- = -(ratio * V+). For 50% tracking ratio, V- = -2.5V.

4. Monitor the power supply output with voltmeter.
    - The voltmeter should read the following values: V+ = 4.95V to 5.05V; V- = -2.55V to -2.45V.

Changing tracking ratio
~~~~~~~~~~~~~~~~~~~~~~~

5. Set tracking ratio to 10%.
    - Negative output Value should be set to -500mV. The voltmeter should read the following values: V+ = 4.95V to 5.05V; V- = -505mV to -495mV.

6. Set tracking ratio to 25%.
    - Negative output Value should be set to -1.25V. The voltmeter should read the following values: V+ = 4.95V to 5.05V; V- = -1.30V to -1.20V.

7. Set tracking ratio to 66%.
    - Negative output Value should be set to -3.3V. The voltmeter should read the following values: V+ = 4.95V to 5.05V; V- = -3.35V to -3.25V.

8. Set tracking ratio to 100%.
    - Negative output Value should be set to -5V. The voltmeter should read the following values: V+ = 4.95V to 5.05V; V- = -5.05V to -4.95V.