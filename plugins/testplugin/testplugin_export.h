
#ifndef TESTPLUGIN_EXPORT_H
#define TESTPLUGIN_EXPORT_H

#ifdef TESTPLUGIN_STATIC_DEFINE
#  define TESTPLUGIN_EXPORT
#  define TESTPLUGIN_NO_EXPORT
#else
#  ifndef TESTPLUGIN_EXPORT
#    ifdef testplugin_EXPORTS
        /* We are building this library */
#      define TESTPLUGIN_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define TESTPLUGIN_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef TESTPLUGIN_NO_EXPORT
#    define TESTPLUGIN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef TESTPLUGIN_DEPRECATED
#  define TESTPLUGIN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef TESTPLUGIN_DEPRECATED_EXPORT
#  define TESTPLUGIN_DEPRECATED_EXPORT TESTPLUGIN_EXPORT TESTPLUGIN_DEPRECATED
#endif

#ifndef TESTPLUGIN_DEPRECATED_NO_EXPORT
#  define TESTPLUGIN_DEPRECATED_NO_EXPORT TESTPLUGIN_NO_EXPORT TESTPLUGIN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef TESTPLUGIN_NO_DEPRECATED
#    define TESTPLUGIN_NO_DEPRECATED
#  endif
#endif

#endif /* TESTPLUGIN_EXPORT_H */
