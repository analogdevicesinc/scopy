
#ifndef SCOPY_DATALOGGERPLUGIN_EXPORT_H
#define SCOPY_DATALOGGERPLUGIN_EXPORT_H

#ifdef SCOPY_DATALOGGERPLUGIN_STATIC_DEFINE
#  define SCOPY_DATALOGGERPLUGIN_EXPORT
#  define SCOPY_DATALOGGERPLUGIN_NO_EXPORT
#else
#  ifndef SCOPY_DATALOGGERPLUGIN_EXPORT
#    ifdef scopy_dataloggerplugin_EXPORTS
        /* We are building this library */
#      define SCOPY_DATALOGGERPLUGIN_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_DATALOGGERPLUGIN_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_DATALOGGERPLUGIN_NO_EXPORT
#    define SCOPY_DATALOGGERPLUGIN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_DATALOGGERPLUGIN_DEPRECATED
#  define SCOPY_DATALOGGERPLUGIN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_DATALOGGERPLUGIN_DEPRECATED_EXPORT
#  define SCOPY_DATALOGGERPLUGIN_DEPRECATED_EXPORT SCOPY_DATALOGGERPLUGIN_EXPORT SCOPY_DATALOGGERPLUGIN_DEPRECATED
#endif

#ifndef SCOPY_DATALOGGERPLUGIN_DEPRECATED_NO_EXPORT
#  define SCOPY_DATALOGGERPLUGIN_DEPRECATED_NO_EXPORT SCOPY_DATALOGGERPLUGIN_NO_EXPORT SCOPY_DATALOGGERPLUGIN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_DATALOGGERPLUGIN_NO_DEPRECATED
#    define SCOPY_DATALOGGERPLUGIN_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_DATALOGGERPLUGIN_EXPORT_H */
