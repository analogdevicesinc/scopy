
#ifndef SCOPY_CORE_EXPORT_H
#define SCOPY_CORE_EXPORT_H

#ifdef SCOPY_CORE_STATIC_DEFINE
#  define SCOPY_CORE_EXPORT
#  define SCOPY_CORE_NO_EXPORT
#else
#  ifndef SCOPY_CORE_EXPORT
#    ifdef scopy_core_EXPORTS
        /* We are building this library */
#      define SCOPY_CORE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_CORE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_CORE_NO_EXPORT
#    define SCOPY_CORE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_CORE_DEPRECATED
#  define SCOPY_CORE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_CORE_DEPRECATED_EXPORT
#  define SCOPY_CORE_DEPRECATED_EXPORT SCOPY_CORE_EXPORT SCOPY_CORE_DEPRECATED
#endif

#ifndef SCOPY_CORE_DEPRECATED_NO_EXPORT
#  define SCOPY_CORE_DEPRECATED_NO_EXPORT SCOPY_CORE_NO_EXPORT SCOPY_CORE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_CORE_NO_DEPRECATED
#    define SCOPY_CORE_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_CORE_EXPORT_H */
