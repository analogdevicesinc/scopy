# About Scripts
Scopy Scripting Guide is available at: https://wiki.analog.com/university/tools/m2k/scopy/scripting-guide

# What's new?

## Scopy Environment-Oriented Commands

1. Accept the license in advance. The command can be used after resetting the license preference or if the application is being used for the first time
	- `scopy.acceptLicense()`
2. Print Scopy about page
	- `scopy.aboutPage()`
3. Start/stop the device scanning process
	- `scopy.startScan(bool scanState)`
		- scanState: True for start, false for stop.
		- return: True if the scan started, false otherwise.
4. Running a script from a given file
	- `scopy.runScript(QString scriptPath, bool exitApp = true)`
		- scriptPath: The path to the script.
		- exitApp: If set to true, the application will be closed after running the given script.
	- `scopy.runScriptList(QStringList scriptPathList, bool exitApp = true)`
		- scriptPathList: A list of paths for multiple scripts.
		- exitApp: If set to true, the application will be closed after running the given script.
5. Get Scopy preferences
	- `scopy.getPreferences()`
		- return: A QMap with pairs of type <QString, QVariant>.
	- `scopy.getPreference(QString prefName)`
		- prefName: The name of the preference.
		- return: A QPair of type <QString, QVariant>.
6. Assign a value to a preference
	- `scopy.setPreference(QString prefName, QVariant value)`
		- prefName: The name of the preference.
		- value: The value to be assigned to the preference.
7. Load a configuration file
	- `scopy.loadSetup(QString filename, QString path)`
		- filename: The name of the configuration file.
		- path: The path to that file (application directory path by default).
		- return: True if the file was loaded, false otherwise.
8. Save the changes of a configuration file
	- `saveSetup(QString filename, QString path)`
		- filename: The name of the configuration file.
		- path: The path where the file should be saved (application directory path by default).
		- return: True if the file was saved, false otherwise.

## Device-Oriented Commands

1. Add a new device to the device browser
	- `scopy.addDevice(QString uri, QString cat = "iio", bool async = false)`
		- uri: Use the context at the provided URI.
		- cat: Device category type ("iio" for example).
		- async: Determine whether the device creation is performed synchronously or asynchronously (false by default).
		- return: The device id on success, an empty QString otherwise.
	- `scopy.addDevice(QString uri, QList<QString> plugins, QString cat = "iio", bool async = false)`
		- uri: Use the context at the provided URI.
		- plugins: Active plugin list.
		- cat: Device category type ("iio" for example).
		- async: Determine whether the device creation is performed synchronously or asynchronously (false by default).
		- return: The device id on success, an empty QString otherwise.
2. Device connection
	- `scopy.connectDevice(int idx)`
		- idx: The index represents the position of the device in the device browser.
		- return: True if the device was successfully connected, false otherwise.
	- `scopy.connectDevice(QString devID)`
		- devID: The device ID that was returned upon adding it to the device browser.
		- return: True if the device was successfully connected, false otherwise.
3. Device disconnection
	- `scopy.disconnectDevice()`
		- Disconnects the last connected device.
		- return: True if the device was successfully disconnected, false otherwise.
	- `scopy.disconnectDevice(QString devID)`
		- devID: The device ID that was returned upon adding it to the device browser.
		- return: True if the device was successfully disconnected, false otherwise.
4. Remove a device from device browser
	- `scopy.removeDevice(int idx)`
		- idx: The index represents the position of the device in the device browser.
		- return: True if the device was successfully removed, false otherwise.
	- `scopy.removeDevice(QString uri, QString cat = "iio")`
	  	- uri: Use the context at the provided URI.
		- cat: Device category type ("iio" for example).
		- return: True if the device was successfully removed, false otherwise.
5. Switching the plugin tools
	- `scopy.switchTool(QString toolName)`
		- toolName: The name of the desired tool from the last connected device.
		- return: True if the tool was successfully switched, false otherwise.
	- `scopy.switchTool(QString devID, QString toolName)`
		- devID: The device ID returned upon adding the device to the device browser.
		- toolName: The name of the desired tool.
		- return: True if the tool was successfully switched, false otherwise.
7. Get the active tools of the most recently connected device
	- `scopy.getTools()`
		- return: A QStringList which contains the device tools.
	- `scopy.getToolsForPlugin(QString plugin)`
		- return: A QStringList containing the `plugin` tools.
8. Get the names of currently connected devices.
	- `scopy.getDevicesName()`
		- return: A QStringList containing the devices.
9. Get the plugin list of a device
	- `scopy.getPlugins(int idx)`
		- idx: The index represents the position of the device in the device browser.
		- return: A QStringList containing the plugins.
	- `scopy.getPlugins(QString uri, QString cat = "iio")`
		- uri: Use the context at the provided URI.
		- cat: Device category type ("iio" for example).
		- return: A QStringList containing the plugins.
10. Start or stop a specific tool of the most recently connected device
	- `scopy.runTool(QString tool, bool flag)`
		- tool: The name of the tool.
		- flag: True if we want the tool to run, false if not.
		- return: True if the running state was changed, false otherwise.
11. Get the current running state of the button
	- `scopy.getToolBtnState(QString tool)`
		- tool: The name of the tool.
		- return: The state of the button on success, false otherwise.
