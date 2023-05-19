
#ifndef SCOPY_DATALOGGER_EXPORT_H
#define SCOPY_DATALOGGER_EXPORT_H

#ifdef SCOPY_DATALOGGER_STATIC_DEFINE
#  define SCOPY_DATALOGGER_EXPORT
#  define SCOPY_DATALOGGER_NO_EXPORT
#else
#  ifndef SCOPY_DATALOGGER_EXPORT
#    ifdef scopy_datalogger_EXPORTS
        /* We are building this library */
#      define SCOPY_DATALOGGER_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_DATALOGGER_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_DATALOGGER_NO_EXPORT
#    define SCOPY_DATALOGGER_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_DATALOGGER_DEPRECATED
#  define SCOPY_DATALOGGER_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_DATALOGGER_DEPRECATED_EXPORT
#  define SCOPY_DATALOGGER_DEPRECATED_EXPORT SCOPY_DATALOGGER_EXPORT SCOPY_DATALOGGER_DEPRECATED
#endif

#ifndef SCOPY_DATALOGGER_DEPRECATED_NO_EXPORT
#  define SCOPY_DATALOGGER_DEPRECATED_NO_EXPORT SCOPY_DATALOGGER_NO_EXPORT SCOPY_DATALOGGER_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_DATALOGGER_NO_DEPRECATED
#    define SCOPY_DATALOGGER_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_DATALOGGER_EXPORT_H */
