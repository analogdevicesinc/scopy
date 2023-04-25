
#ifndef SCOPY_SWIOT_EXPORT_H
#define SCOPY_SWIOT_EXPORT_H

#ifdef SCOPYSWIOT_STATIC_DEFINE
#  define SCOPY_SWIOT_EXPORT
#  define SCOPYSWIOT_NO_EXPORT
#else
#  ifndef SCOPY_SWIOT_EXPORT
#    ifdef scopyswiot_EXPORTS
        /* We are building this library */
#      define SCOPY_SWIOT_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_SWIOT_EXPORT __attribute__((visibility("default")))
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
#  define SCOPYSWIOT_DEPRECATED_EXPORT SCOPY_SWIOT_EXPORT SCOPYSWIOT_DEPRECATED
#endif

#ifndef SCOPYSWIOT_DEPRECATED_NO_EXPORT
#  define SCOPYSWIOT_DEPRECATED_NO_EXPORT SCOPYSWIOT_NO_EXPORT SCOPYSWIOT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYSWIOT_NO_DEPRECATED
#    define SCOPYSWIOT_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_SWIOT_EXPORT_H */
