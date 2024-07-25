#define _EXPAND(A) #A
#define EXPAND(A) _EXPAND(A)

#define DENABLE_INTERNAL_VOLK -DENABLE_INTERNAL_VOLK:BOOL=_ENABLE_VOLK

#ifdef __ARM__
    #define RUNTIME_VERSION 5.14

    #define _ASFLAGS_VALUE -mcpu=cortex-a5 -mfpu=neon-vfpv4 -mfloat-abi=hard
    #define _CFLAGS_VALUE -march=armv7-a -mfpu=neon -mfloat-abi=hard

    #define ASFLAGS ASFLAGS=  _ASFLAGS_VALUE
    #define CFLAGS CFLAGS= _CFLAGS_VALUE

    #define CMAKE_ASM_FLAGS -DCMAKE_ASM_FLAGS= _ASFLAGS_VALUE
    #define CMAKE_C_FLAGS -DCMAKE_C_FLAGS= -fno-asynchronous-unwind-tables _CFLAGS_VALUE

    #define _ENABLE_VOLK OFF

#elif __X86__
    #define RUNTIME_VERSION 5.15-22.08
    #define CFLAGS --disable-assembly
    #define CMAKE_C_FLAGS -DCMAKE_C_FLAGS= -fno-asynchronous-unwind-tables -fPIC
    #define CMAKE_ASM_FLAGS -DCMAKE_ASM_FLAGS=
    #define _ENABLE_VOLK OFF
#endif