#!/bin/bash

set -ex

CLANG_TIDY_CHECKS="-*,performance-*,readability-*,bugprone-*,clang-*,cppcoreguidelines-*,mpi-*,misc-*"
INCLUDED_FILES='*.[ch],*.[ch]xx,*.[ch]pp,*.[ch]++,*.cc,*.hh'
EXCLUDE_PATTERN='*.qss.c'
CLANG_BINARY=clang-tidy-12

install_deps() {
	sudo apt update && sudo apt install ${CLANG_BINARY} gnuradio

	python3 -m pip install --upgrade pip
	pip install requests unidiff PyYAML
}

run_clang_tidy() {
	cp ${GITHUB_WORKSPACE}/build/compile_commands.json ${GITHUB_WORKSPACE}/compile_commands.json

	cd ${GITHUB_WORKSPACE}/CI/github-actions

	patch review.py patchfile.patch

	cd ${GITHUB_WORKSPACE}
	cp ${GITHUB_WORKSPACE}/CI/github-actions/review.py review.py

	python3 review.py --repo=${GITHUB_REPOSITORY} --pr=${GITHUB_PR_NUMBER} --clang_tidy_checks=${CLANG_TIDY_CHECKS} --include=${INCLUDED_FILES} --exclude=${EXCLUDE_PATTERN} --clang_tidy_binary=${CLANG_BINARY}
}

install_deps
run_clang_tidy
