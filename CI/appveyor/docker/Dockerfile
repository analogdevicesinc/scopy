FROM ubuntu:20.04
ENV TZ=Europe/Bucharest
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update -y

# Install base dependencies
RUN apt-get install -y software-properties-common build-essential git sudo apt-utils subversion

# Install flatpak
RUN add-apt-repository ppa:alexlarsson/flatpak -y
RUN apt-get update -y
RUN apt install flatpak flatpak-builder -y

# Install remote
RUN flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
RUN flatpak install flathub org.kde.Platform//5.15 -y
RUN flatpak install flathub org.kde.Sdk//5.15 -y

# Clean
RUN apt-get clean -y && apt-get autoclean -y

RUN echo Cloning scopy-flatpak
ARG REPO_LINK=https://github.com/analogdevicesinc/scopy-flatpak
ARG REPO_LOCAL=/home/docker/scopy-flatpak
ARG REPO_BRANCH=master

RUN git clone --recurse-submodules "$REPO_LINK" -b "$REPO_BRANCH" "$REPO_LOCAL" 2> /dev/null || (cd "$REPO_LOCAL"; git pull)

#RUN cd /home/docker/scopy-flatpak && make -j4
CMD ["/bin/bash"]

