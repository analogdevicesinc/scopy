#!/usr/bin/bash.exe
#set -ex

init_staging() {
	if [ "$USE_STAGING" == "ON" ]
		then
		source mingw_toolchain.sh ON
		rm -rf $STAGING_ENV
		mkdir -p $STAGING_ENV/var/lib/pacman/local
		mkdir -p $STAGING_ENV/var/lib/pacman/sync
		/usr/bin/bash.exe -c "$PACMAN -Syuu bash filesystem mintty pacman"
	fi
}
init_staging
