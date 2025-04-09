
#ifndef SCOPY_IMUANALYZER_EXPORT_H
#define SCOPY_IMUANALYZER_EXPORT_H

#ifdef SCOPY_IMUANALYZER_STATIC_DEFINE
#  define SCOPY_IMUANALYZER_EXPORT
#  define SCOPY_IMUANALYZER_NO_EXPORT
#else
#  ifndef SCOPY_IMUANALYZER_EXPORT
#    ifdef scopy_imuanalyzer_EXPORTS
        /* We are building this library */
#      define SCOPY_IMUANALYZER_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_IMUANALYZER_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_IMUANALYZER_NO_EXPORT
#    define SCOPY_IMUANALYZER_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_IMUANALYZER_DEPRECATED
#  define SCOPY_IMUANALYZER_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_IMUANALYZER_DEPRECATED_EXPORT
#  define SCOPY_IMUANALYZER_DEPRECATED_EXPORT SCOPY_IMUANALYZER_EXPORT SCOPY_IMUANALYZER_DEPRECATED
#endif

#ifndef SCOPY_IMUANALYZER_DEPRECATED_NO_EXPORT
#  define SCOPY_IMUANALYZER_DEPRECATED_NO_EXPORT SCOPY_IMUANALYZER_NO_EXPORT SCOPY_IMUANALYZER_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_IMUANALYZER_NO_DEPRECATED
#    define SCOPY_IMUANALYZER_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_IMUANALYZER_EXPORT_H */
