
#ifndef SCOPYDATALOGGER_EXPORT_H
#define SCOPYDATALOGGER_EXPORT_H

#ifdef SCOPYDATALOGGER_STATIC_DEFINE
#  define SCOPYDATALOGGER_EXPORT
#  define SCOPYDATALOGGER_NO_EXPORT
#else
#  ifndef SCOPYDATALOGGER_EXPORT
#    ifdef scopydatalogger_EXPORTS
        /* We are building this library */
#      define SCOPYDATALOGGER_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYDATALOGGER_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYDATALOGGER_NO_EXPORT
#    define SCOPYDATALOGGER_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYDATALOGGER_DEPRECATED
#  define SCOPYDATALOGGER_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYDATALOGGER_DEPRECATED_EXPORT
#  define SCOPYDATALOGGER_DEPRECATED_EXPORT SCOPYDATALOGGER_EXPORT SCOPYDATALOGGER_DEPRECATED
#endif

#ifndef SCOPYDATALOGGER_DEPRECATED_NO_EXPORT
#  define SCOPYDATALOGGER_DEPRECATED_NO_EXPORT SCOPYDATALOGGER_NO_EXPORT SCOPYDATALOGGER_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYDATALOGGER_NO_DEPRECATED
#    define SCOPYDATALOGGER_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYDATALOGGER_EXPORT_H */
