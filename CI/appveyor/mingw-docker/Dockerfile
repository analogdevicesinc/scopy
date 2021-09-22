# select as base image matching your host to get process isolation
FROM mcr.microsoft.com/windows/servercore:2004

SHELL ["powershell", "-Command", "$ErrorActionPreference = 'Stop'; $ProgressPreference = 'SilentlyContinue';"]
# this creates an empty file named recycle.bin which essentialy disables the recycle bin
# this hack is needed because msys seems to create some files with wierd names in recycle bin
# that freeze the docker layer when commiting
# dirty hack 
RUN echo . > 'C:\$RECYCLE.BIN';  

RUN [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; \
Invoke-WebRequest -UseBasicParsing -uri "https://github.com/msys2/msys2-installer/releases/download/2021-07-25/msys2-base-x86_64-20210725.sfx.exe" -OutFile msys2.exe; \
.\msys2.exe -y -oC:\\; \
Remove-Item msys2.exe; 
RUN 'mkdir C:/msys64/home/docker/ && cd C:/msys64/home/docker/'

SHELL ["cmd", "/S", "/C"]
# set envvars PATH HOME CHERE_INVOKING MSYS_SYSTEM - https://www.msys2.org/docs/ci/#other-systems
RUN setx PATH C:\msys64\bin;C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH% & \
setx HOME C:\msys64\home\docker & \
setx CHERE_INVOKING yes & \
setx MSYSTEM MINGW64 

WORKDIR C:\\msys64\\home\\docker
RUN C:\msys64\usr\bin\bash.exe -lc " "
SHELL ["cmd", "/S", "/C"]
RUN C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Syuu" & C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Syuu" & C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Scc "
# install general packages needed to run build scripts
RUN C:\msys64\usr\bin\bash.exe -c 'pacman --noconfirm --needed -Sy git'
RUN C:\msys64\usr\bin\bash.exe -c "git clone https://github.com/analogdevicesinc/scopy-mingw-build-deps --branch refactor  "

# install packages and build scopy-deps
ARG BUILD_TARGET=x86_64 
ENV BUILD_TARGET=$BUILD_TARGET
RUN echo %BUILD_TARGET%

# exit 0 is required because init_staging force terminates the msys process resulting in an error - it is not an error
RUN C:\msys64\usr\bin\bash.exe -c "cd /home/docker/scopy-mingw-build-deps && ls && echo Building for $BUILD_TARGET &&./init_staging.sh $BUILD_TARGET" & exit 0
RUN C:\msys64\usr\bin\bash.exe -c "cd /home/docker/scopy-mingw-build-deps && source build.sh $BUILD_TARGET && install_tools && install_deps && recurse_submodules"
RUN C:\msys64\home\docker\scopy-mingw-build-deps\is.exe /VERYSILENT /SP- /SUPPRESSMSGBOXES /NORESTART /LOG=C:\msys64\home\docker\iss.log /DIR=C:\innosetup
RUN C:\msys64\usr\bin\bash.exe -c "cd /home/docker/scopy-mingw-build-deps && source build.sh $BUILD_TARGET && build_deps"
# clone scopy

# build scopy



