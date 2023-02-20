
#ifndef SCOPYPLUGINBASE_EXPORT_H
#define SCOPYPLUGINBASE_EXPORT_H

#ifdef SCOPYPLUGINBASE_STATIC_DEFINE
#  define SCOPYPLUGINBASE_EXPORT
#  define SCOPYPLUGINBASE_NO_EXPORT
#else
#  ifndef SCOPYPLUGINBASE_EXPORT
#    ifdef scopypluginbase_EXPORTS
        /* We are building this library */
#      define SCOPYPLUGINBASE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYPLUGINBASE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYPLUGINBASE_NO_EXPORT
#    define SCOPYPLUGINBASE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYPLUGINBASE_DEPRECATED
#  define SCOPYPLUGINBASE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYPLUGINBASE_DEPRECATED_EXPORT
#  define SCOPYPLUGINBASE_DEPRECATED_EXPORT SCOPYPLUGINBASE_EXPORT SCOPYPLUGINBASE_DEPRECATED
#endif

#ifndef SCOPYPLUGINBASE_DEPRECATED_NO_EXPORT
#  define SCOPYPLUGINBASE_DEPRECATED_NO_EXPORT SCOPYPLUGINBASE_NO_EXPORT SCOPYPLUGINBASE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYPLUGINBASE_NO_DEPRECATED
#    define SCOPYPLUGINBASE_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYPLUGINBASE_EXPORT_H */
