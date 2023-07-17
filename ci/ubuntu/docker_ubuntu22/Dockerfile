#docker build -t cristianbindea/scopy2-ubuntu22 .

FROM ubuntu:22.04
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
RUN git clone https://github.com/analogdevicesinc/scopy --branch dev

RUN source scopy/ci/ubuntu/install_ubuntu_22_deps.sh && install_apt

RUN pip3 install aqtinstall && python3 -m aqt install-qt --outputdir $HOME linux desktop 5.15.2

RUN source scopy/ci/ubuntu/install_ubuntu_22_deps.sh && clone
RUN source scopy/ci/ubuntu/install_ubuntu_22_deps.sh && build_deps
RUN sudo rm -rf staging