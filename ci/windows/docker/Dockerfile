# select as base image matching your host to get process isolation
FROM mcr.microsoft.com/windows/servercore:ltsc2019

SHELL ["powershell", "-Command", "$ErrorActionPreference = 'Stop'; $ProgressPreference = 'SilentlyContinue';"]
# this creates an empty file named recycle.bin which essentialy disables the recycle bin
# this hack is needed because msys seems to create some files with wierd names in recycle bin
# that freeze the docker layer when commiting
# dirty hack
RUN echo . > 'C:\$RECYCLE.BIN';

# install msys2 and innosetup
RUN [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; \
    Invoke-WebRequest -UseBasicParsing -uri "https://github.com/msys2/msys2-installer/releases/download/2023-01-27/msys2-base-x86_64-20230127.sfx.exe" -OutFile msys2.exe; \
    .\msys2.exe -y -oC:\\; \
    Remove-Item msys2.exe; \
    Invoke-WebRequest -UseBasicParsing -uri "https://files.jrsoftware.org/is/6/innosetup-6.2.2.exe" -OutFile innosetup-6.2.2.exe; \
    .\innosetup-6.2.2.exe /VERYSILENT /SP- /SUPPRESSMSGBOXES /NORESTART /NOCANCEL /LOG=C:\iss.log /DIR=C:\innosetup;

RUN 'mkdir C:/msys64/home/docker/ && cd C:/msys64/home/docker/'

SHELL ["cmd", "/S", "/C"]
# set envvars PATH HOME CHERE_INVOKING MSYS_SYSTEM - https://www.msys2.org/docs/ci/#other-systems
RUN setx PATH C:\msys64\bin;C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH% & \
    setx HOME C:\msys64\home\docker & \
    setx CHERE_INVOKING yes & \
    setx MSYSTEM MINGW64

ENV CI_SCRIPT=ON

RUN C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Syyu"
RUN C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Sy msys2-keyring"
RUN C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Su"
RUN C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Syuu"
RUN C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Syuu"

WORKDIR C:\\msys64\\home\\docker

COPY mingw_toolchain.sh C:/msys64/home/docker/scripts/mingw_toolchain.sh
COPY windows_build_process.sh C:/msys64/home/docker/scripts/windows_build_process.sh
COPY build_and_create_installer.sh C:/msys64/home/docker/scripts/build_and_create_installer.sh
COPY sigrokdecode-windows-fix.patch C:/msys64/home/docker/scripts/sigrokdecode-windows-fix.patch

RUN C:\msys64\usr\bin\bash.exe -lc "C:/msys64/home/docker/scripts/windows_build_process.sh"
RUN C:\msys64\usr\bin\bash.exe -lc "C:/msys64/home/docker/scripts/build_and_create_installer.sh download_tools"
RUN C:\msys64\usr\bin\bash.exe -lc "mv C:/msys64/home/docker/scripts/staging C:/msys64/home/docker/staging"
RUN C:\msys64\usr\bin\bash.exe -lc "mv C:/msys64/home/docker/scripts/build-status C:/msys64/home/docker/build-status"
RUN C:\msys64\usr\bin\bash.exe -lc "pacman --noconfirm -Scc "
RUN rmdir /s /q C:\msys64\home\docker\scripts
RUN rmdir /s /q C:\msys64\home\docker\staging
