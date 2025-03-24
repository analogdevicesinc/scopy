#!/bin/bash
set -xe
source ./android_toolchain.sh

rm_libs() {
        pushd $ANDROID_SDK_ROOT/ndk/$NDK_VERSION/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/x86_64-linux-android
        rm -f *.so

        if [ $ABI = "armeabi-v7a" ]; then
                find . ! -name 'libunwind.a' -type f -exec rm -f {} +
        else
                rm -f *.a
        fi
	popd
}
create_strip_symlink() {
#needed in NDK r23
#NDK r23 gradle uses wrong binary for stripping so we create a symlink to workaround it
rm -rf $STRIPLINK
ln -s $STRIP $STRIPLINK
}

rm_libs
create_strip_symlink

