cmake_minimum_required(VERSION 3.9)

# Cross-compilation toolchain for armhf (ARM32 hard-float) from x86_64 Uses Ubuntu cross-compiler with Kuiper Linux
# sysroot
#
# Environment variables (set by arm_cross_build_process.sh): CMAKE_SYSROOT    - Path to Kuiper sysroot QT_LOCATION -
# Path to cross-compiled Qt6

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_LIBRARY_ARCHITECTURE arm-linux-gnueabihf)

set(CMAKE_SYSROOT $ENV{CMAKE_SYSROOT})
set(QT_LOCATION $ENV{QT_LOCATION})

# Cross-compiler (Ubuntu multiarch)
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(CMAKE_AR arm-linux-gnueabihf-ar)
set(CMAKE_ASM_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_LINKER arm-linux-gnueabihf-ld)
set(CMAKE_OBJCOPY arm-linux-gnueabihf-objcopy)
set(CMAKE_RANLIB arm-linux-gnueabihf-ranlib)
set(CMAKE_SIZE arm-linux-gnueabihf-size)
set(CMAKE_STRIP arm-linux-gnueabihf-strip)

# Compiler flags
set(CMAKE_C_FLAGS "-march=armv7-a -mfloat-abi=hard -mfpu=neon -fPIC")
set(CMAKE_ASM_FLAGS "-march=armv7-a -mfloat-abi=hard -mfpu=neon")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SYSROOT}/include")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SYSROOT}/usr/include")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SYSROOT}/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}")
set(CMAKE_CXX_FLAGS "-fexceptions -frtti ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-Os -g")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,-O1 -Wl,--hash-style=gnu")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_SYSROOT}/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_SYSROOT}/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_SYSROOT}/usr/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_SYSROOT}/usr/local/lib")
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})

# Search paths
set(CMAKE_PREFIX_PATH "${QT_LOCATION}")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SYSROOT}/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SYSROOT}/usr/lib")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SYSROOT}/usr/local/lib")

# pkg-config for cross-compilation
set(ENV{PKG_CONFIG_ALLOW_CROSS} 1)
set(ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}")
set(ENV{PKG_CONFIG_LIBDIR}
    "${CMAKE_SYSROOT}/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig:${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig:${CMAKE_SYSROOT}/usr/local/lib/pkgconfig"
)
set(ENV{PKG_CONFIG_PATH} "${CMAKE_SYSROOT}/usr/include:${CMAKE_SYSROOT}/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}")

# Find root path modes
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT} ${QT_LOCATION})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# RPATH for AppImage portability
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
list(APPEND CMAKE_INSTALL_RPATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
list(APPEND CMAKE_INSTALL_RPATH "/usr/lib")

set(CMAKE_NO_SYSTEM_FROM_IMPORTED TRUE)
