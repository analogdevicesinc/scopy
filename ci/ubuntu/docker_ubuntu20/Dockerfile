#docker build -t cristianbindea/scopy2-ubuntu22 --build-arg QT_USER=<your_user> --build-arg QT_PASSWORD=<your_password> .

FROM ubuntu:20.04
SHELL ["/bin/bash", "-c"]

ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive

ENV TZ=Europe/Bucharest
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && apt-get -y upgrade && apt-get install -y apt-utils sudo git

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
RUN git clone https://github.com/analogdevicesinc/scopy --branch scopy2_docker_ubuntu

RUN source scopy/ci/ubuntu/install_ubuntu_20_deps.sh && install_apt

RUN wget https://download.qt.io/official_releases/online_installers/qt-unified-linux-x64-online.run && chmod +x qt-unified-linux-x64-online.run
ARG QT_USER
ARG QT_PASSWORD
RUN ./qt-unified-linux-x64-online.run install --ao --al --aa DownloadError=Retry -c -m $QT_USER --pw $QT_PASSWORD qt.qt5.5152.gcc_64 qt.tools.cmake
RUN sudo apt remove qtchooser && sudo ln -s /home/$USER/Qt/Tools/QtCreator/bin/qtcreator.sh /usr/local/bin/qtcreator

RUN source scopy/ci/ubuntu/install_ubuntu_20_deps.sh && clone
RUN source scopy/ci/ubuntu/install_ubuntu_20_deps.sh && build_deps
RUN sudo rm -rf staging