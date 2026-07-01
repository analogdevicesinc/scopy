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
	tar -czf "$CI_ARM_DIR/kuiper-rootfs.tar.gz" \
		--exclude='./proc' --exclude='./sys' --exclude='./dev' \
		--exclude='./run' --exclude='./tmp' --exclude='./mnt' \
		--exclude='./media' --exclude='./home/*/staging' \
		--exclude='./home/*/scopy' --exclude='./boot' \
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
	echo "Next steps:"
	echo "  1. docker build --tag docker.cloudsmith.io/adi/scopy-dockers/scopy2-arm64-native-qt6:testing -f ci/arm/docker/Dockerfile.arm64 ci/arm"
	echo "  2. sudo docker login docker.cloudsmith.io -u token -p \$CLOUDSMITH_API_KEY"
	echo "  3. sudo docker push docker.cloudsmith.io/adi/scopy-dockers/scopy2-arm64-native-qt6:testing"
}

if [ $# -eq 0 ]; then
	run_all
else
	for arg in "$@"; do
		$arg
	done
fi
