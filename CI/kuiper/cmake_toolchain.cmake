cmake_minimum_required(VERSION 3.18)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_LIBRARY_ARCHITECTURE arm-linux-gnueabihf)

# In this case the variables CMAKE_SYSROOT and STAGING_AREA are defined as parameters to the cmake command
set(TOOLCHAIN_FILE ${STAGING_AREA}/cross-pi-gcc)
set(TOOLCHAIN_BIN ${TOOLCHAIN_FILE}/bin)
set(CMAKE_PREFIX_PATH ${QT_LOCATION})
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SYSROOT}/lib")
set(CMAKE_VERBOSE ON)

set(PKG_CONFIG_EXECUTABLE "${CMAKE_SYSROOT}/usr/bin/arm-linux-gnueabihf-pkg-config" CACHE PATH "PKG_CONFIG_EXECUTABLE"
											  FORCE
)
set(ENV{PKG_CONFIG_ALLOW_CROSS} 1)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
set(RPI_PKG_CONFIG_LIBDIR "${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf/pkgconfig:${RPI_PKG_CONFIG_LIBDIR}")
set(RPI_PKG_CONFIG_LIBDIR "${CMAKE_SYSROOT}/usr/share/pkgconfig:${RPI_PKG_CONFIG_LIBDIR}")
set(RPI_PKG_CONFIG_LIBDIR "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${RPI_PKG_CONFIG_LIBDIR}")
set(RPI_PKG_CONFIG_LIBDIR "${CMAKE_SYSROOT}/usr/local/lib/pkgconfig:${RPI_PKG_CONFIG_LIBDIR}")
set(ENV{PKG_CONFIG_LIBDIR} "${RPI_PKG_CONFIG_LIBDIR}")
set(ENV{PKG_CONFIG} "${CMAKE_SYSROOT}/usr/bin/arm-linux-gnueabihf-pkg-config")

set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${CMAKE_SYSROOT}/include:")
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${CMAKE_SYSROOT}/usr/include:")
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${CMAKE_SYSROOT}/usr/include/arm-linux-gnueabihf:")
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${CMAKE_SYSROOT}/usr/share/include:")

set(ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}")
set(ENV{LD_LIBRARY_PATH} "${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf:$ENV{LD_LIBRARY_PATH}")

set(CMAKE_LIBRARY_PATH "${CMAKE_SYSROOT}/usr/lib")
set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH} ${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf")
set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH} ${CMAKE_SYSROOT}/usr/local/lib")
set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH} ${CMAKE_SYSROOT}/usr/local/lib/arm-linux-gnueabihf")

set(CMAKE_AR ${TOOLCHAIN_BIN}/arm-linux-gnueabihf-ar)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN}/arm-linux-gnueabihf-gcc)
set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN}/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN}/arm-linux-gnueabihf-g++)
set(CMAKE_LINKER ${TOOLCHAIN_BIN}/arm-linux-gnueabihf-ld)
set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN}/arm-linux-gnueabihf-objcopy)
set(CMAKE_RANLIB ${TOOLCHAIN_BIN}/arm-linux-gnueabihf-ranlib)
set(CMAKE_SIZE ${TOOLCHAIN_BIN}/arm-linux-gnueabihf-size)
set(CMAKE_STRIP ${TOOLCHAIN_BIN}/arm-linux-gnueabihf-strip)

set(CMAKE_C_FLAGS "-march=armv7-a -mfloat-abi=hard -mfpu=vfp")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SYSROOT}/include")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SYSROOT}/usr/include")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SYSROOT}/usr/include/arm-linux-gnueabihf")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SYSROOT}/usr/share/include")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "-fexceptions -frtti ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-Os -g")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,-O1 -Wl,--hash-style=gnu -mthumb -lpthread -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_SYSROOT}/usr/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_SYSROOT}/usr/local/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${QT_LOCATION}/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_FILE}/arm-linux-gnueabihf/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN_FILE}/arm-linux-gnueabihf/libc/lib")
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})

#[[
# Debug Mode
set(PKG_CONFIG_ARGN "--debug")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--verbose ")
]]

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY) # Perform compiler test with static library
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH
    "$ORIGIN"
    "$ORIGIN/../lib"
    "/usr/lib/arm-linux-gnueabihf"
    "/lib/arm-linux-gnueabihf"
    "/lib"
    "/usr/local/qt5.15/lib"
)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
