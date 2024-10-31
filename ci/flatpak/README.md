# Scopy Flatpak Recipe
Flatpak build recipe for Scopy

Scopy is a software oscilloscope and signal analysis toolset. [The official repository](https://github.com/analogdevicesinc/scopy) provides releases for Windows, Linux, macOS and Android.
This recipe is used to build the Linux Flatpak installer. 

### Install Scopy using the Flatpak release for Linux using these instructions:
 - [Install Flatpak using these instructions](http://flatpak.org/getting.html)
 - [Download Scopy.flatpak from the Releases Tab](https://github.com/analogdevicesinc/scopy/releases)
 - Install the application using ``` flatpak install Scopy.flatpak ```
 - Run the application using ``` flatpak run org.adi.Scopy ```

## Building the Docker image
To build the Docker image just execute the ***create_docker.sh*** script.

The provided Dockerfile will install the KDE Runtime over the Ubuntu 20.04 base image. This Docker image is built in such a way that it contains the dependencies needed for the packaging of the Scopy application. It leverages the Flatpak Builder caching system, where after each step in the build process the result is saved as cache in order to be reused for later builds.

To build the Flatpak package for Scopy inside this Docker image, it needs to be run using ***--privileged***, otherwise there is a lack of access to necessary utilities.

## Generating the Scopy.flatpak artifact
Running ```make``` inside the ***scopy/ci/flatpak*** folder will build the Scopy.flatpak artifact.