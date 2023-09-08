
#ifndef SCOPY_ADCPLUGIN_EXPORT_H
#define SCOPY_ADCPLUGIN_EXPORT_H

#ifdef SCOPY_ADCPLUGIN_STATIC_DEFINE
#  define SCOPY_ADCPLUGIN_EXPORT
#  define SCOPY_ADCPLUGIN_NO_EXPORT
#else
#  ifndef SCOPY_ADCPLUGIN_EXPORT
#    ifdef scopy_adcplugin_EXPORTS
        /* We are building this library */
#      define SCOPY_ADCPLUGIN_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_ADCPLUGIN_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_ADCPLUGIN_NO_EXPORT
#    define SCOPY_ADCPLUGIN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_ADCPLUGIN_DEPRECATED
#  define SCOPY_ADCPLUGIN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_ADCPLUGIN_DEPRECATED_EXPORT
#  define SCOPY_ADCPLUGIN_DEPRECATED_EXPORT SCOPY_ADCPLUGIN_EXPORT SCOPY_ADCPLUGIN_DEPRECATED
#endif

#ifndef SCOPY_ADCPLUGIN_DEPRECATED_NO_EXPORT
#  define SCOPY_ADCPLUGIN_DEPRECATED_NO_EXPORT SCOPY_ADCPLUGIN_NO_EXPORT SCOPY_ADCPLUGIN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_ADCPLUGIN_NO_DEPRECATED
#    define SCOPY_ADCPLUGIN_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_ADCPLUGIN_EXPORT_H */
