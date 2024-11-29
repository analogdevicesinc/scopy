#!/bin/bash 
set -e

NUM_JOBS=4
WORKDIR="${PWD}/deps"
mkdir -p "$WORKDIR"
if [ "$TRAVIS" == "true" ] ; then
	STAGINGDIR=/usr/local
else
	STAGINGDIR="${WORKDIR}/staging"
fi

export PYTHON3=python3

version_gt() { test "$(echo "$@" | tr " " "\n" | sort -V | head -n 1)" != "$1"; }
version_le() { test "$(echo "$@" | tr " " "\n" | sort -V | head -n 1)" == "$1"; }
version_lt() { test "$(echo "$@" | tr " " "\n" | sort -rV | head -n 1)" != "$1"; }
version_ge() { test "$(echo "$@" | tr " " "\n" | sort -rV | head -n 1)" == "$1"; }

get_codename() {
	lsb_release -c -s
}

get_dist_id() {
	lsb_release -i -s
}

get_version() {
	lsb_release -r -s
}

is_new_ubuntu() {
	[ "$(get_dist_id)" == "Ubuntu" ] || return 1
	version_ge "$(get_version)" "18.04"
}

TRAVIS_BUILD_DIR=${TRAVIS_BUILD_DIR:-'./'}

LIBIIO_BRANCH=master

command_exists() {
	local cmd=$1
	[ -n "$cmd" ] || return 1
	type "$cmd" >/dev/null 2>&1
}

ensure_command_exists() {
	local cmd="$1"
	local package="$2"
	[ -n "$cmd" ] || return 1
	[ -n "$package" ] || package="$cmd"
	! command_exists "$cmd" || return 0
	# go through known package managers
	for pacman in apt-get brew yum ; do
		command_exists $pacman || continue
		$pacman install -y $package || {
			# Try an update if install doesn't work the first time
			$pacman -y update && \
				$pacman install -y $package
		}
		return $?
	done
	return 1
}

ensure_command_exists wget
ensure_command_exists sudo

# Get the common stuff from libiio
[ -f ${TRAVIS_BUILD_DIR}/build/lib.sh ] || {
	mkdir -p ${TRAVIS_BUILD_DIR}/build
	wget https://raw.githubusercontent.com/analogdevicesinc/libiio/master/CI/travis/lib.sh \
		-O ${TRAVIS_BUILD_DIR}/build/lib.sh
}

. ${TRAVIS_BUILD_DIR}/build/lib.sh

if [ -z "${LDIST}" -a -f "build/.LDIST" ] ; then
	export LDIST="-$(cat build/.LDIST)"
fi
if [ -z "${LDIST}" ] || [ "$LDIST" = "DO_NOT_DEPLOY" ] ; then
	export LDIST="-$(get_ldist)"
fi
