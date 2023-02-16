
#ifndef SCOPYCORE_EXPORT_H
#define SCOPYCORE_EXPORT_H

#ifdef SCOPYCORE_STATIC_DEFINE
#  define SCOPYCORE_EXPORT
#  define SCOPYCORE_NO_EXPORT
#else
#  ifndef SCOPYCORE_EXPORT
#    ifdef scopycore_EXPORTS
        /* We are building this library */
#      define SCOPYCORE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYCORE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYCORE_NO_EXPORT
#    define SCOPYCORE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYCORE_DEPRECATED
#  define SCOPYCORE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYCORE_DEPRECATED_EXPORT
#  define SCOPYCORE_DEPRECATED_EXPORT SCOPYCORE_EXPORT SCOPYCORE_DEPRECATED
#endif

#ifndef SCOPYCORE_DEPRECATED_NO_EXPORT
#  define SCOPYCORE_DEPRECATED_NO_EXPORT SCOPYCORE_NO_EXPORT SCOPYCORE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYCORE_NO_DEPRECATED
#    define SCOPYCORE_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYCORE_EXPORT_H */
