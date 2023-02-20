
#ifndef SCOPYPLUGIN_EXPORT_H
#define SCOPYPLUGIN_EXPORT_H

#ifdef SCOPYPLUGIN_STATIC_DEFINE
#  define SCOPYPLUGIN_EXPORT
#  define SCOPYPLUGIN_NO_EXPORT
#else
#  ifndef SCOPYPLUGIN_EXPORT
#    ifdef scopyplugin_EXPORTS
        /* We are building this library */
#      define SCOPYPLUGIN_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYPLUGIN_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYPLUGIN_NO_EXPORT
#    define SCOPYPLUGIN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYPLUGIN_DEPRECATED
#  define SCOPYPLUGIN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYPLUGIN_DEPRECATED_EXPORT
#  define SCOPYPLUGIN_DEPRECATED_EXPORT SCOPYPLUGIN_EXPORT SCOPYPLUGIN_DEPRECATED
#endif

#ifndef SCOPYPLUGIN_DEPRECATED_NO_EXPORT
#  define SCOPYPLUGIN_DEPRECATED_NO_EXPORT SCOPYPLUGIN_NO_EXPORT SCOPYPLUGIN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYPLUGIN_NO_DEPRECATED
#    define SCOPYPLUGIN_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYPLUGIN_EXPORT_H */
