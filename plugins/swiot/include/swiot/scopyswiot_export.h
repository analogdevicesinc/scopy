
#ifndef SCOPYSWIOT_EXPORT_H
#define SCOPYSWIOT_EXPORT_H

#ifdef SCOPYSWIOT_STATIC_DEFINE
#  define SCOPYSWIOT_EXPORT
#  define SCOPYSWIOT_NO_EXPORT
#else
#  ifndef SCOPYSWIOT_EXPORT
#    ifdef scopyswiot_EXPORTS
        /* We are building this library */
#      define SCOPYSWIOT_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYSWIOT_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYSWIOT_NO_EXPORT
#    define SCOPYSWIOT_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYSWIOT_DEPRECATED
#  define SCOPYSWIOT_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYSWIOT_DEPRECATED_EXPORT
#  define SCOPYSWIOT_DEPRECATED_EXPORT SCOPYSWIOT_EXPORT SCOPYSWIOT_DEPRECATED
#endif

#ifndef SCOPYSWIOT_DEPRECATED_NO_EXPORT
#  define SCOPYSWIOT_DEPRECATED_NO_EXPORT SCOPYSWIOT_NO_EXPORT SCOPYSWIOT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYSWIOT_NO_DEPRECATED
#    define SCOPYSWIOT_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYSWIOT_EXPORT_H */
