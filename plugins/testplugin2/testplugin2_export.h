
#ifndef TESTPLUGIN2_EXPORT_H
#define TESTPLUGIN2_EXPORT_H

#ifdef TESTPLUGIN2_STATIC_DEFINE
#  define TESTPLUGIN2_EXPORT
#  define TESTPLUGIN2_NO_EXPORT
#else
#  ifndef TESTPLUGIN2_EXPORT
#    ifdef testplugin2_EXPORTS
        /* We are building this library */
#      define TESTPLUGIN2_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define TESTPLUGIN2_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef TESTPLUGIN2_NO_EXPORT
#    define TESTPLUGIN2_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef TESTPLUGIN2_DEPRECATED
#  define TESTPLUGIN2_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef TESTPLUGIN2_DEPRECATED_EXPORT
#  define TESTPLUGIN2_DEPRECATED_EXPORT TESTPLUGIN2_EXPORT TESTPLUGIN2_DEPRECATED
#endif

#ifndef TESTPLUGIN2_DEPRECATED_NO_EXPORT
#  define TESTPLUGIN2_DEPRECATED_NO_EXPORT TESTPLUGIN2_NO_EXPORT TESTPLUGIN2_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef TESTPLUGIN2_NO_DEPRECATED
#    define TESTPLUGIN2_NO_DEPRECATED
#  endif
#endif

#endif /* TESTPLUGIN2_EXPORT_H */
