#!/bin/sh -xe

if [ -n "$BRANCH" ]; then
	ARTIFACT_LOCATION=/scopy
else
	ARTIFACT_LOCATION=$GITHUB_WORKSPACE
fi

apt-get install -y jq

REPO_LOCAL=/home/docker/scopy-flatpak
cd "$REPO_LOCAL"
# this ensures that latest master is pulled from origin while keeping file cache
# the cache should be updated from time to time locally
git fetch && git reset origin/master --hard

# Run the preprocess step to generate org.adi.Scopy.json
make preprocess

# Disable the preprocess step; The Json file will now be modified and
# we don't want to re-generate it at the build step
export EN_PREPROCESS=false

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

# Generate build status info for the about page
echo "Details about the versions of dependencies can be found <a href="https://github.com/analogdevicesinc/scopy-flatpak/blob/master/org.adi.Scopy.json.c">here</a>" > build-status
cp build-status $GITHUB_WORKSPACE/build-status

# Insert env vars in the sandboxed flatpak build
CI_ENVS=`jq -R -n -c '[inputs|split("=")|{(.[0]):.[1]}] | add' $GITHUB_WORKSPACE/CI/appveyor/gh-actions.envs`
echo "CI_ENVS= $CI_ENVS"
cat org.adi.Scopy.json | jq --tab '."build-options".env += ('$CI_ENVS')' > tmp.json
cp tmp.json org.adi.Scopy.json

make clean
make -j4

# Copy the Scopy.flatpak file in $GITHUB_WORKSPACE (which is the external location, mount when docker starts)
cp Scopy.flatpak $ARTIFACT_LOCATION/
