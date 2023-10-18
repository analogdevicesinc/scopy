# Install Linux distribution in WSL2

- From a PowerShell
  ```powershell
    wsl --install -d Ubuntu-22.04 --set-version 2 
  ```


# Steps - run in WSL2

1. Clone Scopy repo
``` bash
    git clone https://github.com/analogdevicesinc/scopy.git -b ubuntu/wsl2_build 
```

2. Install **aqt** -> taken from ci/ubuntu/docker_ubuntu22/Dockerfile
``` bash
pip3 install aqtinstall && python3 -m aqt install-qt --outputdir $HOME linux desktop 5.15.2
```

3. Install qwt deps
- Run script from **ci/ubuntu/wsl_ubuntu22_build.sh**

## Note: known issues

### * libcuda.so and libcuda.so.1 should be symlink
  - https://github.com/microsoft/WSL/issues/5663#issuecomment-1068499676
  - https://forums.developer.nvidia.com/t/wsl2-libcuda-so-and-libcuda-so-1-should-be-symlink/236301

4. Run the built application
NOTE: in WSL2 there is a server (WSLg) that allows to run GUI applications directly.
``` bash
    cd scopy/build-x86_64
    ./scopy
```

# Attach M2k to WSL2 using USB interface
- Follow [this tutorial](https://learn.microsoft.com/en-us/windows/wsl/connect-usb). You then should be able to connect USB devices to WSL2

TODO: usb udev rules cmd that shoulb be used inside wsl


NOTE:
- mix cu sters din cmd si creat link din wsl