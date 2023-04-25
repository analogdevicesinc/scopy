
#ifndef SCOPY_SWIOT_EXPORT_H
#define SCOPY_SWIOT_EXPORT_H

#ifdef SCOPY_SWIOT_STATIC_DEFINE
#  define SCOPY_SWIOT_EXPORT
#  define SCOPY_SWIOT_NO_EXPORT
#else
#  ifndef SCOPY_SWIOT_EXPORT
#    ifdef scopy_swiot_EXPORTS
        /* We are building this library */
#      define SCOPY_SWIOT_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_SWIOT_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_SWIOT_NO_EXPORT
#    define SCOPY_SWIOT_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_SWIOT_DEPRECATED
#  define SCOPY_SWIOT_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_SWIOT_DEPRECATED_EXPORT
#  define SCOPY_SWIOT_DEPRECATED_EXPORT SCOPY_SWIOT_EXPORT SCOPY_SWIOT_DEPRECATED
#endif

#ifndef SCOPY_SWIOT_DEPRECATED_NO_EXPORT
#  define SCOPY_SWIOT_DEPRECATED_NO_EXPORT SCOPY_SWIOT_NO_EXPORT SCOPY_SWIOT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_SWIOT_NO_DEPRECATED
#    define SCOPY_SWIOT_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_SWIOT_EXPORT_H */
