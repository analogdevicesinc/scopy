
#ifndef SCOPY_M2KPLUGIN_EXPORT_H
#define SCOPY_M2KPLUGIN_EXPORT_H

#ifdef SCOPY_M2KPLUGIN_STATIC_DEFINE
#  define SCOPY_M2KPLUGIN_EXPORT
#  define SCOPY_M2KPLUGIN_NO_EXPORT
#else
#  ifndef SCOPY_M2KPLUGIN_EXPORT
#    ifdef scopy_m2kplugin_EXPORTS
        /* We are building this library */
#      define SCOPY_M2KPLUGIN_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_M2KPLUGIN_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_M2KPLUGIN_NO_EXPORT
#    define SCOPY_M2KPLUGIN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_M2KPLUGIN_DEPRECATED
#  define SCOPY_M2KPLUGIN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_M2KPLUGIN_DEPRECATED_EXPORT
#  define SCOPY_M2KPLUGIN_DEPRECATED_EXPORT SCOPY_M2KPLUGIN_EXPORT SCOPY_M2KPLUGIN_DEPRECATED
#endif

#ifndef SCOPY_M2KPLUGIN_DEPRECATED_NO_EXPORT
#  define SCOPY_M2KPLUGIN_DEPRECATED_NO_EXPORT SCOPY_M2KPLUGIN_NO_EXPORT SCOPY_M2KPLUGIN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_M2KPLUGIN_NO_DEPRECATED
#    define SCOPY_M2KPLUGIN_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_M2KPLUGIN_EXPORT_H */