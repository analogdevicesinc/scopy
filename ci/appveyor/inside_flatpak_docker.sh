#!/bin/sh -xe

if [ -n "$BRANCH" ]; then
	ARTIFACT_LOCATION=/scopy
else
	ARTIFACT_LOCATION=$GITHUB_WORKSPACE
fi

apt-get install -y jq

REPO_LOCAL=/home/docker/scopy-flatpak
cd "$REPO_LOCAL"
git pull && git checkout master

# check the number of elements in the json file in order to get the last element, which is Scopy
cnt=$( echo `jq '.modules | length' org.adi.Scopy.json` )
cnt=$(($cnt-1))

if [ -n "$BRANCH" ]; then
	REPO_URL=https://github.com/"$REPO"
	# We are building in Appveyor and we have access to the current branch on a CACHED Docker image
	# use jq to replace the Scopy branch + the repo url used for building
	# we want to build the branch and repo we're currently on
	cat org.adi.Scopy.json | jq --tab '.modules['$cnt'].sources[0].branch = "'$BRANCH'"' > tmp.json
	cp tmp.json org.adi.Scopy.json
	cat org.adi.Scopy.json | jq --tab '.modules['$cnt'].sources[0].url = "'$REPO_URL'"' > tmp.json
else
	# We are building in Github Actions and we use the current directory folder on a CLEAN Docker image
	cat org.adi.Scopy.json | jq --tab '.modules['$cnt'].sources[0].type = "dir"' > tmp.json
	cp tmp.json org.adi.Scopy.json
	cat org.adi.Scopy.json | jq --tab '.modules['$cnt'].sources[0].path = "'$GITHUB_WORKSPACE'"' > tmp.json
	cp tmp.json org.adi.Scopy.json
	cat org.adi.Scopy.json | jq --tab 'del(.modules['$cnt'].sources[0].url)' > tmp.json
	cp tmp.json org.adi.Scopy.json
	cat org.adi.Scopy.json | jq --tab 'del(.modules['$cnt'].sources[0].branch)' > tmp.json
fi
cp tmp.json org.adi.Scopy.json
rm tmp.json

make clean
make -j4

# Copy the Scopy.flatpak file in $GITHUB_WORKSPACE (which is the external location, mount when docker starts)
cp Scopy.flatpak $ARTIFACT_LOCATION/
