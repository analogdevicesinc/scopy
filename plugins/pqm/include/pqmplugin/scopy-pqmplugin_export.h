
#ifndef SCOPY_PQMPLUGIN_EXPORT_H
#define SCOPY_PQMPLUGIN_EXPORT_H

#ifdef SCOPY_PQMPLUGIN_STATIC_DEFINE
#  define SCOPY_PQMPLUGIN_EXPORT
#  define SCOPY_PQMPLUGIN_NO_EXPORT
#else
#  ifndef SCOPY_PQMPLUGIN_EXPORT
#    ifdef scopy_pqmplugin_EXPORTS
        /* We are building this library */
#      define SCOPY_PQMPLUGIN_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_PQMPLUGIN_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_PQMPLUGIN_NO_EXPORT
#    define SCOPY_PQMPLUGIN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_PQMPLUGIN_DEPRECATED
#  define SCOPY_PQMPLUGIN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_PQMPLUGIN_DEPRECATED_EXPORT
#  define SCOPY_PQMPLUGIN_DEPRECATED_EXPORT SCOPY_PQMPLUGIN_EXPORT SCOPY_PQMPLUGIN_DEPRECATED
#endif

#ifndef SCOPY_PQMPLUGIN_DEPRECATED_NO_EXPORT
#  define SCOPY_PQMPLUGIN_DEPRECATED_NO_EXPORT SCOPY_PQMPLUGIN_NO_EXPORT SCOPY_PQMPLUGIN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_PQMPLUGIN_NO_DEPRECATED
#    define SCOPY_PQMPLUGIN_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_PQMPLUGIN_EXPORT_H */
