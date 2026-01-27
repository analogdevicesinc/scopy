.. _imuanalyzer:

IMU Analyzer
================================================================================

General Description
---------------------------------------------------------------------

The IMU Analyzer plugin provides comprehensive analysis and visualization capabilities for Inertial Measurement Unit (IMU) sensor data. This plugin transforms complex IMU sensor readings into intuitive 3D visualizations and detailed measurement displays, making it an essential tool for orientation analysis, motion tracking, and sensor validation.

The IMU Analyzer automatically detects ADIS family IMU sensors and provides real-time 3D orientation visualization, multi-axis sensor monitoring, and professional-grade measurement tools. The plugin features an interactive 3D scene renderer that displays device orientation in real-time, combined with detailed numerical data panels for precise analysis.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/imuanalyzer/imuanalyzer.png
    :align: center




.. note::
   The IMU Analyzer requires compatible ADIS family IMU hardware and OpenGL-capable graphics hardware for optimal performance.

.. warning::
   High sampling rates (>1000 Hz) may require significant system resources. Monitor system performance and adjust sampling frequency as needed.

Key Features
---------------------------------------------------------------------

**3D Orientation Visualization**
  * **Real-time 3D Display**: Interactive OpenGL-based 3D scene showing device orientation
  * **Interactive Controls**: Pan, zoom, and rotate view with mouse controls
  * **View Reset**: One-click return to default viewing angle

**Multi-Axis Sensor Monitoring**
  * **3-Axis Gyroscope**: Angular velocity measurements (X, Y, Z axes)
  * **3-Axis Accelerometer**: Linear acceleration data with gravity compensation
  * **Delta Measurements**: Advanced delta angle and velocity calculations

**Advanced Data Analysis**
  * **Measurement Mode**: Detailed numerical data panels with real-time updates
  * **Multi-Tab Interface**: Organized view switching between visualization modes
  * **Data Visualizer**: Comprehensive sensor data display with formatted output
  * **Real-time Processing**: Threaded data acquisition for smooth operation


Hardware Requirements
---------------------------------------------------------------------

**Compatible Devices**
  The IMU Analyzer plugin is designed for ADIS family IMU sensors from Analog Devices:

  * **ADIS16550**: High-performance tactical-grade IMU
  * **ADIS16545**: Precision industrial IMU
  * **ADIS16547**: Six degrees of freedom IMU
  * **ADIS16505**: Low-cost MEMS IMU
  * **Other ADIS Devices**: Any device with "adis" in the IIO device name

**Primary Target: ADIS16550**
  The plugin is optimized for the ADIS16550 tactical-grade IMU featuring:

**Software Dependencies**
  * Scopy v2.2.0 or later
  * OpenGL support for 3D visualization
  * IIO drivers for ADIS devices

**Connection Requirements**
  * USB or Ethernet connection to ADIS evaluation board
  * Stable IIO context for device communication


Using the IMU Analyzer
---------------------------------------------------------------------

**Connecting to an IMU Device**

1. **Hardware Setup**

   * Connect your ADIS IMU evaluation board via USB or Ethernet
   * Ensure the device is powered and recognized by the system
   * Verify IIO drivers are installed and functional

2. **Device Detection**

   * Launch Scopy and use device discovery
   * Connect to your ADIS device - the IMU Analyzer will automatically activate
   * The plugin appears in the tool list when compatible hardware is detected

3. **Tool Interface**

   * Click on "IMUAnalyzer" in the tool menu
   * The interface opens with the 3D View tab active
   * Control buttons appear in the tool header

**3D Visualization Operation**

1. **Starting Data Acquisition**

   * Click the **Run** button to start real-time data collection
   * The 3D scene begins displaying device orientation immediately
   * Rotate or move your IMU device to see real-time updates

2. **3D View Navigation**

   * **Mouse Controls**: Drag to rotate view, scroll to zoom
   * **View Reset**: Click "Reset View" to return to default angle
   * **Real-time Updates**: Orientation changes reflect device movement instantly

3. **Understanding the 3D Display**

   * The 3D object represents your IMU device orientation
   * X, Y, Z axes are color-coded for easy identification
   * Smooth animation shows orientation changes in real-time

**Measurement Mode**

1. **Enabling Measurements**

   * Click the **Measure** button to toggle measurement panel
   * Detailed numerical data appears alongside 3D visualization
   * Data updates in real-time with current sensor readings

2. **Data Panel Information**

   * **Rotation Data**: Current orientation angles (pitch, roll, yaw)
   * **Position Data**: Calculated position based on acceleration integration
   * **Temperature**: Current sensor temperature for compensation
   * **Raw Sensor Values**: Direct access to gyroscope and accelerometer readings

3. **Data Interpretation**

   * **Angular Velocity**: Current rotation rates in rad/s
   * **Acceleration**: Linear acceleration including gravity (m/s²)
   * **Delta Values**: Change in angle and velocity since last sample
   * **Temperature**: Sensor temperature for thermal compensation


Settings and Configuration
---------------------------------------------------------------------

**Sensor Settings**
  Access through the settings panel:

  * **Sampling Frequency**: Adjust data acquisition rate 
  * **Filter Configuration**: Low-pass filter settings for noise reduction
  * **Calibration Parameters**: Bias and scale factor adjustments
  * **Buffer Settings**: Data buffer configuration for high-speed operation

