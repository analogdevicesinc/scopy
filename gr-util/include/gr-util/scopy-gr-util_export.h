
#ifndef SCOPY_GR_UTIL_EXPORT_H
#define SCOPY_GR_UTIL_EXPORT_H

#ifdef SCOPY_GR_UTIL_STATIC_DEFINE
#  define SCOPY_GR_UTIL_EXPORT
#  define SCOPY_GR_UTIL_NO_EXPORT
#else
#  ifndef SCOPY_GR_UTIL_EXPORT
#    ifdef scopy_gr_util_EXPORTS
        /* We are building this library */
#      define SCOPY_GR_UTIL_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_GR_UTIL_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_GR_UTIL_NO_EXPORT
#    define SCOPY_GR_UTIL_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_GR_UTIL_DEPRECATED
#  define SCOPY_GR_UTIL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_GR_UTIL_DEPRECATED_EXPORT
#  define SCOPY_GR_UTIL_DEPRECATED_EXPORT SCOPY_GR_UTIL_EXPORT SCOPY_GR_UTIL_DEPRECATED
#endif

#ifndef SCOPY_GR_UTIL_DEPRECATED_NO_EXPORT
#  define SCOPY_GR_UTIL_DEPRECATED_NO_EXPORT SCOPY_GR_UTIL_NO_EXPORT SCOPY_GR_UTIL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_GR_UTIL_NO_DEPRECATED
#    define SCOPY_GR_UTIL_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_GR_UTIL_EXPORT_H */
