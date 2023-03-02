
#ifndef SCOPYIIOUTIL_EXPORT_H
#define SCOPYIIOUTIL_EXPORT_H

#ifdef SCOPYIIOUTIL_STATIC_DEFINE
#  define SCOPYIIOUTIL_EXPORT
#  define SCOPYIIOUTIL_NO_EXPORT
#else
#  ifndef SCOPYIIOUTIL_EXPORT
#    ifdef scopyiioutil_EXPORTS
        /* We are building this library */
#      define SCOPYIIOUTIL_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYIIOUTIL_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYIIOUTIL_NO_EXPORT
#    define SCOPYIIOUTIL_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYIIOUTIL_DEPRECATED
#  define SCOPYIIOUTIL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYIIOUTIL_DEPRECATED_EXPORT
#  define SCOPYIIOUTIL_DEPRECATED_EXPORT SCOPYIIOUTIL_EXPORT SCOPYIIOUTIL_DEPRECATED
#endif

#ifndef SCOPYIIOUTIL_DEPRECATED_NO_EXPORT
#  define SCOPYIIOUTIL_DEPRECATED_NO_EXPORT SCOPYIIOUTIL_NO_EXPORT SCOPYIIOUTIL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYIIOUTIL_NO_DEPRECATED
#    define SCOPYIIOUTIL_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYIIOUTIL_EXPORT_H */
