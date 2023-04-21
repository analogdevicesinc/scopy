
#ifndef SCOPYREGMAP_EXPORT_H
#define SCOPYREGMAP_EXPORT_H

#ifdef SCOPYREGMAP_STATIC_DEFINE
#  define SCOPYREGMAP_EXPORT
#  define SCOPYREGMAP_NO_EXPORT
#else
#  ifndef SCOPYREGMAP_EXPORT
#    ifdef scopyregmap_EXPORTS
        /* We are building this library */
#      define SCOPYREGMAP_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYREGMAP_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYREGMAP_NO_EXPORT
#    define SCOPYREGMAP_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYREGMAP_DEPRECATED
#  define SCOPYREGMAP_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYREGMAP_DEPRECATED_EXPORT
#  define SCOPYREGMAP_DEPRECATED_EXPORT SCOPYREGMAP_EXPORT SCOPYREGMAP_DEPRECATED
#endif

#ifndef SCOPYREGMAP_DEPRECATED_NO_EXPORT
#  define SCOPYREGMAP_DEPRECATED_NO_EXPORT SCOPYREGMAP_NO_EXPORT SCOPYREGMAP_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYREGMAP_NO_DEPRECATED
#    define SCOPYREGMAP_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYREGMAP_EXPORT_H */
