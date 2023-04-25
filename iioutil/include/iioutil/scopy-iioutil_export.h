
#ifndef SCOPY_IIOUTIL_EXPORT_H
#define SCOPY_IIOUTIL_EXPORT_H

#ifdef SCOPY_IIOUTIL_STATIC_DEFINE
#  define SCOPY_IIOUTIL_EXPORT
#  define SCOPY_IIOUTIL_NO_EXPORT
#else
#  ifndef SCOPY_IIOUTIL_EXPORT
#    ifdef scopy_iioutil_EXPORTS
        /* We are building this library */
#      define SCOPY_IIOUTIL_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_IIOUTIL_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_IIOUTIL_NO_EXPORT
#    define SCOPY_IIOUTIL_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_IIOUTIL_DEPRECATED
#  define SCOPY_IIOUTIL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_IIOUTIL_DEPRECATED_EXPORT
#  define SCOPY_IIOUTIL_DEPRECATED_EXPORT SCOPY_IIOUTIL_EXPORT SCOPY_IIOUTIL_DEPRECATED
#endif

#ifndef SCOPY_IIOUTIL_DEPRECATED_NO_EXPORT
#  define SCOPY_IIOUTIL_DEPRECATED_NO_EXPORT SCOPY_IIOUTIL_NO_EXPORT SCOPY_IIOUTIL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_IIOUTIL_NO_DEPRECATED
#    define SCOPY_IIOUTIL_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_IIOUTIL_EXPORT_H */
