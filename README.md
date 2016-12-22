[![appveyor status](https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva/branch/master?svg=true)](https://ci.appveyor.com/project/analogdevicesinc/scopy/branch/master)

# Scopy
Scopy is a software oscilloscope and signal analysis toolset.

## Appveyor Builds 
Nightly builds for Windows are available at: https://ci.appveyor.com/project/analogdevicesinc/scopy/build/artifacts

Each build contains files that allow two types of installations:
- System install:  The scopy-setup.exe installs Scopy and all required dependencies. Recommended for a quick and easy installation.
- Local install: To install, extract scopy.zip which contains Scopy dependencies and copy scopy.exe next to the extracted files.
Local installs can be used when there is a need for multiple versions of Scopy to exist simultaneously on a system or when new changes are made only to the scopy.exe file and there is no need for the scopy.zip to be downloaded again.
