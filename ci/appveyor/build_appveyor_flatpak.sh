#!/bin/bash

set -m

BRANCH=${1:-master}
REPO=${2:-analogdevicesinc/scopy}

sudo apt-get -qq update
sudo service docker restart
sudo docker pull alexandratr/scopy-flatpak-bionic:latest

# Start the docker in detached state
commit_nb=$(sudo docker run -d \
		--privileged \
		--rm=true \
		-v `pwd`:/scopy:rw \
		-e "BRANCH=$BRANCH" \
		-e "REPO=$REPO" \
		alexandratr/scopy-flatpak-bionic:latest \
		/bin/bash -xe /scopy/ci/appveyor/inside_flatpak_docker.sh)

# Attach ourselves to the running docker and wait for it to finish
sudo docker attach $commit_nb

