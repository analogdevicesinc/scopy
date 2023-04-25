
#ifndef SCOPY_TESTPLUGIN2_EXPORT_H
#define SCOPY_TESTPLUGIN2_EXPORT_H

#ifdef SCOPY_TESTPLUGIN2_STATIC_DEFINE
#  define SCOPY_TESTPLUGIN2_EXPORT
#  define SCOPY_TESTPLUGIN2_NO_EXPORT
#else
#  ifndef SCOPY_TESTPLUGIN2_EXPORT
#    ifdef scopy_testplugin2_EXPORTS
        /* We are building this library */
#      define SCOPY_TESTPLUGIN2_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_TESTPLUGIN2_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_TESTPLUGIN2_NO_EXPORT
#    define SCOPY_TESTPLUGIN2_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_TESTPLUGIN2_DEPRECATED
#  define SCOPY_TESTPLUGIN2_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_TESTPLUGIN2_DEPRECATED_EXPORT
#  define SCOPY_TESTPLUGIN2_DEPRECATED_EXPORT SCOPY_TESTPLUGIN2_EXPORT SCOPY_TESTPLUGIN2_DEPRECATED
#endif

#ifndef SCOPY_TESTPLUGIN2_DEPRECATED_NO_EXPORT
#  define SCOPY_TESTPLUGIN2_DEPRECATED_NO_EXPORT SCOPY_TESTPLUGIN2_NO_EXPORT SCOPY_TESTPLUGIN2_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_TESTPLUGIN2_NO_DEPRECATED
#    define SCOPY_TESTPLUGIN2_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_TESTPLUGIN2_EXPORT_H */
