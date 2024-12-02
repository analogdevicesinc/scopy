#!/bin/sh
set -e

cd $TRAVIS_BUILD_DIR

send()
{
if [ "$#" -ne 3 ] ; then
	echo "skipping deployment of something"
        echo "send called with $@"
	return
fi

if [ "x$1" = "x" ] ; then
	echo no file to send
	return
fi

if [ ! -r "$1" ] ; then
	echo "file $1 is not readable"
	ls -l $1
	return
fi

if [ $BRANCH_PULL ] ; then
	branch=$BRANCH_PULL
else
	branch=$BRANCH
fi

FROM=$1
TO=${branch}_$2
LATE=${branch}_latest_scopy${LDIST}$3
GLOB=${DEPLOY_TO}/${branch}_scopy-*

echo attemting to deploy $FROM to $TO
echo and ${branch}_scopy${LDIST}$3
ssh -V

echo "cd ${DEPLOY_TO}" > script$3
if curl -m 10 -s -I -f -o /dev/null http://swdownloads.analog.com/cse/travis_builds/${TO} ; then
	echo "rm ${TO}" >> script$3
fi
echo "put ${FROM} ${TO}" >> script$3
echo "ls -l ${TO}" >> script$3
if curl -m 10 -s -I -f -o /dev/null http://swdownloads.analog.com/cse/travis_builds/${LATE} ; then
	echo "rm ${LATE}" >> script$3
fi
echo "symlink ${TO} ${LATE}" >> script$3
echo "ls -l ${LATE}" >> script$3
echo "bye" >> script$3

sftp ${EXTRA_SSH} -b script$3 ${SSHUSER}@${SSHHOST}

# limit things to a few files, so things don't grow forever
if [ "${LDIST}" = "-precise" -a "$3" = ".deb" ] ; then
	for files in $(ssh ${EXTRA_SSH} ${SSHUSER}@${SSHHOST} \
		"ls -lt ${GLOB}" | tail -n +100 | awk '{print $NF}')
	do
		ssh ${EXTRA_SSH} ${SSHUSER}@${SSHHOST} \
			"rm ${DEPLOY_TO}/${files}"
	done
fi
}

#    from                   to             suffix
send ${RELEASE_PKG_FILE_DMG} ${TARGET_DMG} .dmg
