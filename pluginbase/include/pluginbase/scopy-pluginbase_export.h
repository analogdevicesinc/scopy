
#ifndef SCOPY_PLUGINBASE_EXPORT_H
#define SCOPY_PLUGINBASE_EXPORT_H

#ifdef SCOPY_PLUGINBASE_STATIC_DEFINE
#  define SCOPY_PLUGINBASE_EXPORT
#  define SCOPY_PLUGINBASE_NO_EXPORT
#else
#  ifndef SCOPY_PLUGINBASE_EXPORT
#    ifdef scopy_pluginbase_EXPORTS
        /* We are building this library */
#      define SCOPY_PLUGINBASE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_PLUGINBASE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_PLUGINBASE_NO_EXPORT
#    define SCOPY_PLUGINBASE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_PLUGINBASE_DEPRECATED
#  define SCOPY_PLUGINBASE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_PLUGINBASE_DEPRECATED_EXPORT
#  define SCOPY_PLUGINBASE_DEPRECATED_EXPORT SCOPY_PLUGINBASE_EXPORT SCOPY_PLUGINBASE_DEPRECATED
#endif

#ifndef SCOPY_PLUGINBASE_DEPRECATED_NO_EXPORT
#  define SCOPY_PLUGINBASE_DEPRECATED_NO_EXPORT SCOPY_PLUGINBASE_NO_EXPORT SCOPY_PLUGINBASE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_PLUGINBASE_NO_DEPRECATED
#    define SCOPY_PLUGINBASE_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_PLUGINBASE_EXPORT_H */
