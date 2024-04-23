#docker build -t cristianbindea/scopy1-ubuntu-appimage .
#DOCKER_BUILDKIT=0
FROM ubuntu:20.04
SHELL ["/bin/bash", "-c"]

ARG USER=runner
ARG DEBIAN_FRONTEND=noninteractive

ENV TZ=Europe/Bucharest
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && apt-get -y upgrade && apt-get install -y apt-utils sudo git tzdata keyboard-configuration

RUN groupadd -g 1000 -r $USER && \
	useradd -u 1000 -g 1000 --create-home -r $USER

#Change password
RUN echo "$USER:$USER" | chpasswd

#Make sudo passwordless
RUN echo "${USER} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/90-$USER && \
	usermod -aG sudo $USER && \
	usermod -aG plugdev $USER

USER $USER
WORKDIR /home/${USER}
RUN git clone https://github.com/analogdevicesinc/scopy --branch appimage-scopy1
WORKDIR /home/${USER}/scopy

RUN ./CI/appimage/ubuntu_appimage_process.sh install_packages
RUN sudo pip3 install aqtinstall && sudo python3 -m aqt install-qt --outputdir /opt/Qt linux desktop 5.15.2
RUN ./CI/appimage/ubuntu_appimage_process.sh clone download_tools build_deps

# to be set at runtime CI_SCRIPT=ON
WORKDIR /home/${USER}
# RUN rm -rf scopy
