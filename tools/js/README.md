# About Scripts
Scopy Scripting Guide is available at: https://wiki.analog.com/university/tools/m2k/scopy/scripting-guide

# New features (Scopy2)

1. Adding a new device in device browser
	- Command: `scopy.addDevice(QString category, QString uri)`
		- category: Device category type ("iio" for example)
		- uri: Use the context at the provided URI
2. Establishing a connection with a device
	- Commands:
		- `scopy.connectDevice(int idx)`
			- idx: The index represents the position of the device in the device browser
		- `scopy.connectDevice(QString devID)`
			- devID: The device ID that was returned upon adding it to the device browser
3. Device disconnection
	- Commands:
		- `scopy.disconnectDevice()`
			- Disconnects the last connected device
		- `scopy.disconnectDevice(QString devID)`
			- devID: The device ID that was returned upon adding it to the device browser
4. Switching the plugin tools
	- Each plugin includes a list of tools, allowing you to choose which one to be the current tool
	- Commands:
		- `scopy.switchTool(QString toolName)`
			- toolName: The name of the desired tool from the last connected device
		- `scopy.switchTool(QString devID, QString toolName)`
			- devID: The device ID that was returned upon adding it to the device browser
			- toolName: The name of the desired tool
5. Running a script from a given file
	- Command: `scopy.runScript(QString scriptPath, bool exitApp = true)`
		- scriptPath: The path to the script
		- exitApp: If set to true, the application will be closed after running the given script
