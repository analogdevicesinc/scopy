
#ifndef SCOPY_COMMON_EXPORT_H
#define SCOPY_COMMON_EXPORT_H

#ifdef SCOPY_COMMON_STATIC_DEFINE
#  define SCOPY_COMMON_EXPORT
#  define SCOPY_COMMON_NO_EXPORT
#else
#  ifndef SCOPY_COMMON_EXPORT
#    ifdef scopy_common_EXPORTS
        /* We are building this library */
#      define SCOPY_COMMON_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_COMMON_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_COMMON_NO_EXPORT
#    define SCOPY_COMMON_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_COMMON_DEPRECATED
#  define SCOPY_COMMON_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_COMMON_DEPRECATED_EXPORT
#  define SCOPY_COMMON_DEPRECATED_EXPORT SCOPY_COMMON_EXPORT SCOPY_COMMON_DEPRECATED
#endif

#ifndef SCOPY_COMMON_DEPRECATED_NO_EXPORT
#  define SCOPY_COMMON_DEPRECATED_NO_EXPORT SCOPY_COMMON_NO_EXPORT SCOPY_COMMON_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_COMMON_NO_DEPRECATED
#    define SCOPY_COMMON_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_COMMON_EXPORT_H */
