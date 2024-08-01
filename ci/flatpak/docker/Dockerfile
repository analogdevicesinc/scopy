FROM ubuntu:20.04
CMD ["/bin/bash"]
ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Bucharest
ENV CI_SCRIPT=ON
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install starting packages
RUN apt update && apt -y upgrade && \
	apt install -y apt-utils software-properties-common sudo

# Create user
RUN groupadd -g 1000 -r $USER && \
	useradd -u 1000 -g 1000 --create-home -r $USER

# Change password
RUN echo "$USER:$USER" | chpasswd

# Make sudo passwordless
RUN echo "${USER} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/90-$USER && \
	usermod -aG sudo $USER && \
	usermod -aG plugdev $USER

USER $USER

# Install base dependencies
# Install flatpak
# Clean apt
RUN sudo apt install -y git wget vim build-essential subversion mm-common && \
	sudo apt install flatpak jq flatpak-builder -y && \
	sudo apt clean -y && sudo apt autoclean -y

# Install remote
RUN sudo flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
RUN sudo flatpak install flathub org.kde.Platform//5.15-23.08 -y && sudo flatpak install flathub org.kde.Sdk//5.15-23.08 -y

WORKDIR /home/$USER
