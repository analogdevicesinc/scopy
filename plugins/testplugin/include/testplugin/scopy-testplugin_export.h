
#ifndef SCOPY_TESTPLUGIN_EXPORT_H
#define SCOPY_TESTPLUGIN_EXPORT_H

#ifdef SCOPY_TESTPLUGIN_STATIC_DEFINE
#  define SCOPY_TESTPLUGIN_EXPORT
#  define SCOPY_TESTPLUGIN_NO_EXPORT
#else
#  ifndef SCOPY_TESTPLUGIN_EXPORT
#    ifdef scopy_testplugin_EXPORTS
        /* We are building this library */
#      define SCOPY_TESTPLUGIN_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_TESTPLUGIN_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_TESTPLUGIN_NO_EXPORT
#    define SCOPY_TESTPLUGIN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_TESTPLUGIN_DEPRECATED
#  define SCOPY_TESTPLUGIN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_TESTPLUGIN_DEPRECATED_EXPORT
#  define SCOPY_TESTPLUGIN_DEPRECATED_EXPORT SCOPY_TESTPLUGIN_EXPORT SCOPY_TESTPLUGIN_DEPRECATED
#endif

#ifndef SCOPY_TESTPLUGIN_DEPRECATED_NO_EXPORT
#  define SCOPY_TESTPLUGIN_DEPRECATED_NO_EXPORT SCOPY_TESTPLUGIN_NO_EXPORT SCOPY_TESTPLUGIN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_TESTPLUGIN_NO_DEPRECATED
#    define SCOPY_TESTPLUGIN_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_TESTPLUGIN_EXPORT_H */
