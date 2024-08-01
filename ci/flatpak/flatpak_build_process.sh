#!/bin/bash -xe

SCOPY_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || \
SCOPY_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && cd ../../ && pwd )

SCOPY_JSON=$SCOPY_DIR/ci/flatpak/org.adi.Scopy.json

if [ "$CI_SCRIPT" == "ON" ];
	then
		SOURCE_DIR=$GITHUB_WORKSPACE
		# this is needed in order to be used by flatpak caching system
		# the docker image already contains the built dependencies so we just have to move them
		cp -r /home/runner/flatpak_tools/.flatpak-builder $SOURCE_DIR/ci/flatpak
		cp -r /home/runner/flatpak_tools/build $SOURCE_DIR/ci/flatpak
	else
		SOURCE_DIR=$SCOPY_DIR
fi

pushd $SCOPY_DIR/ci/flatpak

git submodule update --init

# Run the preprocess step to generate org.adi.Scopy.json
make preprocess

# Disable the preprocess step; The Json file will now be modified and
# we don't want to re-generate it at the build step
export EN_PREPROCESS=false

# check the number of elements in the json file in order to get the last element, which is Scopy
cnt=$( echo $(jq '.modules | length' $SCOPY_JSON) )
cnt=$(($cnt-1))

# We are building in Github Actions and we use the current directory folder on a CLEAN Docker image
cat $SCOPY_JSON | jq --tab '.modules['$cnt'].sources[0].type = "dir"' > tmp.json
cp tmp.json $SCOPY_JSON
cat $SCOPY_JSON | jq --tab '.modules['$cnt'].sources[0].path = "'$SOURCE_DIR'"' > tmp.json
cp tmp.json $SCOPY_JSON
cat $SCOPY_JSON | jq --tab 'del(.modules['$cnt'].sources[0].url)' > tmp.json
cp tmp.json $SCOPY_JSON
cat $SCOPY_JSON | jq --tab 'del(.modules['$cnt'].sources[0].branch)' > tmp.json
cp tmp.json $SCOPY_JSON
rm tmp.json

# Generate build status info for the about page
echo "Details about the versions of dependencies can be found <a href="https://github.com/analogdevicesinc/scopy-flatpak/blob/master/org.adi.Scopy.json.c">here</a>" > build-status
cp build-status $SOURCE_DIR/build-status

# Insert env vars in the sandboxed flatpak build
# $SOURCE_DIR/ci/general/gen_ci_envs.sh > $SOURCE_DIR/ci/general/gh-actions.envs
# CI_ENVS=$(jq -R -n -c '[inputs|split("=")|{(.[0]):.[1]}] | add' $SOURCE_DIR/ci/general/gh-actions.envs)
# echo "CI_ENVS= $CI_ENVS"
# cat $SCOPY_JSON | jq --tab '."build-options".env += ('$CI_ENVS')' > tmp.json
# cp tmp.json $SCOPY_JSON

make

# Copy the Scopy.flatpak file in $SOURCE_DIR (which is the external location, mount when docker starts)
[ -z $CI_SCRIPT ] || cp Scopy.flatpak $SOURCE_DIR/
popd
