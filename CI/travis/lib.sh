#!/bin/bash

DEPSDIR="${PWD}/deps"
STAGINGDIR="${DEPSDIR}/staging"

echo_red()   { printf "\033[1;31m$*\033[m\n"; }

get_codename() {
	lsb_release -a 2>/dev/null | grep Codename | cut -f2
}

is_new_ubuntu() {
	[ "$(uname -s)" == "Linux" ] || return 1
	[ "$(get_codename)" == "bionic" ]
}
