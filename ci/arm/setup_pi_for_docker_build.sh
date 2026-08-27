#!/bin/bash
set -ex

# Prepare a fresh Kuiper Linux ARM64 Pi for building the Scopy ARM64 Docker image.
# Usage: sudo ./setup_pi_for_docker_build.sh          (runs all steps)
#        sudo ./setup_pi_for_docker_build.sh step_name (runs a specific step)
#
# Prerequisites:
#   - Raspberry Pi 4/5 booted with ADI Kuiper Linux ARM64
#   - 64 GB+ SD card
#   - Internet connection

SRC_DIR=$(git rev-parse --show-toplevel 2>/dev/null) || \
SRC_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && cd ../../ && pwd)

CI_ARM_DIR="$SRC_DIR/ci/arm"

fix_clock() {
	echo "=== Fixing system clock ==="
	if ! date | grep -q "$(date -u +%Y)"; then
		echo "System clock appears incorrect. Setting to current UTC time from internet..."
		apt-get install -y ntpdate 2>/dev/null || true
		ntpdate -u pool.ntp.org 2>/dev/null || \
		date -s "$(wget -qO- http://worldtimeapi.org/api/ip.txt 2>/dev/null | grep datetime | cut -d' ' -f2 | cut -dT -f1,2 | tr T ' ')" 2>/dev/null || \
		echo "WARNING: Could not auto-set clock. Set manually: sudo date -s 'YYYY-MM-DD HH:MM:SS'"
	fi
	echo "Current date: $(date)"
}

fix_apt_mirror() {
	echo "=== Fixing apt mirror (deb.debian.org CDN can be stale for Trixie) ==="
	sed -i 's|http://deb.debian.org|http://ftp.debian.org|g' /etc/apt/sources.list 2>/dev/null || true
	sed -i 's|http://deb.debian.org|http://ftp.debian.org|g' /etc/apt/sources.list.d/*.list 2>/dev/null || true
	apt-get clean
	apt-get update
}

upgrade_system() {
	echo "=== Upgrading system packages ==="
	# apt-get update is required, not optional: dist-upgrade compares against the cached package
	# lists, so without a refresh it reports "0 upgraded" on a system that is months behind and
	# silently does nothing. run_all gets this via fix_apt_mirror, but this step is also run
	# standalone before create_rootfs_tarball - and a stale base rootfs makes the image's own
	# `apt-get -y upgrade` (Dockerfile.arm64) re-download every outdated package into a new layer
	# while the base layer keeps the superseded copies, which cost ~3 GB in the first slim build.
	apt-get update
	apt-get -y dist-upgrade || true
	apt --fix-broken install -y || true
}

install_docker() {
	echo "=== Installing Docker ==="
	if command -v docker &>/dev/null; then
		echo "Docker already installed: $(docker --version)"
		return
	fi
	curl -fsSL https://get.docker.com | sh
	usermod -aG docker analog 2>/dev/null || true
}

install_build_deps() {
	echo "=== Installing build dependencies ==="
	$CI_ARM_DIR/arm_native_build_process.sh install_packages
}

build_qt6() {
	echo "=== Building Qt6 from source (this takes 2-3 hours) ==="
	$CI_ARM_DIR/arm_native_build_process.sh download_cmake
	$CI_ARM_DIR/arm_native_build_process.sh download_qt6_source
	$CI_ARM_DIR/arm_native_build_process.sh configure_qt6
	$CI_ARM_DIR/arm_native_build_process.sh compile_qt6
	$CI_ARM_DIR/arm_native_build_process.sh install_qt6
}

build_scopy_deps() {
	echo "=== Building Scopy dependencies ==="
	$CI_ARM_DIR/arm_native_build_process.sh clone
	$CI_ARM_DIR/arm_native_build_process.sh build_deps
}

create_qt6_tarball() {
	echo "=== Creating Qt6 tarball for Docker ==="
	tar -czf "$CI_ARM_DIR/qt6-arm64-installed.tar.gz" -C / opt/Qt/6.8.3/gcc_arm64
	echo "Created: $CI_ARM_DIR/qt6-arm64-installed.tar.gz ($(du -h "$CI_ARM_DIR/qt6-arm64-installed.tar.gz" | cut -f1))"
}

create_rootfs_tarball() {
	echo "=== Creating rootfs tarball for Docker ==="
	# /usr/local and /opt/Qt are excluded deliberately. Dockerfile.arm64 rebuilds the whole of
	# /usr/local via `clone` + `build_deps`, and installs Qt from qt6-arm64-installed.tar.gz, so
	# capturing them here would bake the *previous* dependency generation into the image's base
	# layer - where copy-deps.sh, which searches /usr/local/lib first, could pull a stale library
	# into the AppImage. run_all makes this a certainty rather than a risk: build_scopy_deps runs
	# immediately before this function.
	# ./home/* is excluded wholesale - the image creates its own `runner` user (Dockerfile.arm64)
	# and never reads the builder's home directory.
	# ./var/log/* is the builder machine's own logs - 328 MB of them, of no use to a CI image.
	# The trailing /* on these three keeps the directory itself, which the image still needs.
	tar -czf "$CI_ARM_DIR/kuiper-rootfs.tar.gz" \
		--exclude='./proc' --exclude='./sys' --exclude='./dev' \
		--exclude='./run' --exclude='./tmp' --exclude='./mnt' \
		--exclude='./media' --exclude='./boot' \
		--exclude='./home/*' --exclude='./var/log/*' \
		--exclude='./usr/local/*' --exclude='./opt/Qt' \
		--exclude='./var/cache/apt' --exclude='./var/lib/docker' \
		--exclude='./var/lib/containerd' \
		-C / .
	echo "Created: $CI_ARM_DIR/kuiper-rootfs.tar.gz ($(du -h "$CI_ARM_DIR/kuiper-rootfs.tar.gz" | cut -f1))"
}

cleanup_build_artifacts() {
	echo "=== Cleaning up build artifacts to free disk space ==="
	STAGING="$(dirname "$CI_ARM_DIR")/staging"
	[ -d "$HOME/staging" ] && rm -rf "$HOME/staging"
	echo "Freed space. Current: $(df -h / | tail -1 | awk '{print $4}') available"
}

run_all() {
	fix_clock
	fix_apt_mirror
	upgrade_system
	install_docker
	install_build_deps
	build_qt6
	build_scopy_deps
	create_qt6_tarball
	create_rootfs_tarball
	cleanup_build_artifacts
	echo ""
	echo "=== Setup complete ==="
	echo "Next steps (TAG selects the image variant; the dependency-rework pass uses slim):"
	echo "  1. sudo TAG=slim $CI_ARM_DIR/create_docker_image_qt6.sh arm64"
	echo "  2. sudo docker login docker.cloudsmith.io -u token"
	echo "  3. sudo docker push docker.cloudsmith.io/adi/scopy-dockers/scopy2-arm64-native-qt6:slim"
}

if [ $# -eq 0 ]; then
	run_all
else
	for arg in "$@"; do
		$arg
	done
fi
