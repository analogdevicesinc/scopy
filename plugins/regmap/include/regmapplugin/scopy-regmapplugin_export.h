
#ifndef SCOPY_REGMAPPLUGIN_EXPORT_H
#define SCOPY_REGMAPPLUGIN_EXPORT_H

#ifdef SCOPY_REGMAPPLUGIN_STATIC_DEFINE
#  define SCOPY_REGMAPPLUGIN_EXPORT
#  define SCOPY_REGMAPPLUGIN_NO_EXPORT
#else
#  ifndef SCOPY_REGMAPPLUGIN_EXPORT
#    ifdef scopy_regmapplugin_EXPORTS
        /* We are building this library */
#      define SCOPY_REGMAPPLUGIN_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_REGMAPPLUGIN_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_REGMAPPLUGIN_NO_EXPORT
#    define SCOPY_REGMAPPLUGIN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_REGMAPPLUGIN_DEPRECATED
#  define SCOPY_REGMAPPLUGIN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_REGMAPPLUGIN_DEPRECATED_EXPORT
#  define SCOPY_REGMAPPLUGIN_DEPRECATED_EXPORT SCOPY_REGMAPPLUGIN_EXPORT SCOPY_REGMAPPLUGIN_DEPRECATED
#endif

#ifndef SCOPY_REGMAPPLUGIN_DEPRECATED_NO_EXPORT
#  define SCOPY_REGMAPPLUGIN_DEPRECATED_NO_EXPORT SCOPY_REGMAPPLUGIN_NO_EXPORT SCOPY_REGMAPPLUGIN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_REGMAPPLUGIN_NO_DEPRECATED
#    define SCOPY_REGMAPPLUGIN_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_REGMAPPLUGIN_EXPORT_H */
