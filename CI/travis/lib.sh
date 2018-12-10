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

echo_red()   { printf "\033[1;31m$*\033[m\n"; }

get_codename() {
	lsb_release -a 2>/dev/null | grep Codename | cut -f2
}

is_new_ubuntu() {
	[ "$(uname -s)" == "Linux" ] || return 1
	[ "$(get_codename)" == "bionic" ]
}

get_ldist() {
	case "$(uname)" in
	Linux*)
		if [ ! -f /etc/os-release ] ; then
			if [ -f /etc/centos-release ] ; then
				echo "centos-$(sed -e 's/CentOS release //' -e 's/(.*)$//' \
					-e 's/ //g' /etc/centos-release)-$(uname -m)"
				return 0
			fi
			ls /etc/*elease
			[ -z "${OSTYPE}" ] || {
				echo "${OSTYPE}-unknown"
				return 0
			}
			echo "linux-unknown"
			return 0
		fi
		. /etc/os-release
		if ! command dpkg --version >/dev/null 2>&1 ; then
			echo $ID-$VERSION_ID-$(uname -m)
		else
			echo $ID-$VERSION_ID-$(dpkg --print-architecture)
		fi
		;;
	Darwin*)
		echo "darwin-$(sw_vers -productVersion)"
		;;
	*)
		echo "$(uname)-unknown"
		;;
	esac
	return 0
}
