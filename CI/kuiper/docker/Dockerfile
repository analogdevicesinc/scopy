FROM --platform=linux/amd64 ubuntu:20.04 AS start
SHELL ["/bin/bash", "-c"]
ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Bucharest
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && \
	apt-get -y upgrade && \
	apt-get install -y apt-utils sudo git wget flex bison pkg-config make python3 pip vim
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



FROM start AS sysroot_builder
ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive
COPY sysroot.tar.gz /home/${USER}
RUN git clone https://github.com/analogdevicesinc/scopy -b dev
WORKDIR /home/${USER}/scopy
RUN ./CI/kuiper/create_sysroot.sh \
		install_packages \
		move_and_extract_sysroot \
		fix_relativelinks
RUN ./CI/kuiper/build_qt.sh \
		install_packages \
		download_qt \
		download_crosscompiler \
		build_qt5.15.2
RUN ./CI/kuiper/kuiper_build_process.sh \
		install_packages \
		download_cmake \
		download_crosscompiler \
		clone \
		build_deps



FROM start
ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive
RUN mkdir -p /home/${USER}/sysroot
COPY --from=sysroot_builder /home/${USER}/scopy/CI/kuiper/staging/sysroot/usr /home/${USER}/sysroot/usr
COPY --from=sysroot_builder /home/${USER}/scopy/CI/kuiper/staging/sysroot/share /home/${USER}/sysroot/share
COPY --from=sysroot_builder /home/${USER}/scopy/CI/kuiper/staging/sysroot/include /home/${USER}/sysroot/include
WORKDIR /home/${USER}/sysroot
RUN ln -s usr/bin bin && ln -s usr/lib lib && ln -s usr/sbin sbin
WORKDIR /home/${USER}