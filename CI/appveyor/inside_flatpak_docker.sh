#!/bin/sh -xe

apt-get install -y jq

REPO_URL=https://github.com/"$REPO"
REPO_LOCAL=/home/docker/scopy-flatpak
cd "$REPO_LOCAL"
git checkout master && git pull

# check the number of elements in the json file in order to get the last element, which is Scopy
cnt=$( echo `jq '.modules | length' org.adi.Scopy.json` )
cnt=$(($cnt-1))

# use jq to replace the Scopy branch + the repo url used for building
# we want to build the branch and repo we're currently on
cat org.adi.Scopy.json | jq --tab '.modules['$cnt'].sources[0].branch = "'$BRANCH'"' > tmp.json
cp tmp.json org.adi.Scopy.json
cat org.adi.Scopy.json | jq --tab '.modules['$cnt'].sources[0].url = "'$REPO_URL'"' > tmp.json
cp tmp.json org.adi.Scopy.json
rm tmp.json

make clean
make -j4

# Copy the Scopy.flatpak file in /scopy (which is the external location, mount when docker starts)
cp Scopy.flatpak /scopy/
