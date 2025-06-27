
#ifndef SCOPY_PKG_MANAGER_EXPORT_H
#define SCOPY_PKG_MANAGER_EXPORT_H

#ifdef SCOPY_PKG_MANAGER_STATIC_DEFINE
#  define SCOPY_PKG_MANAGER_EXPORT
#  define SCOPY_PKG_MANAGER_NO_EXPORT
#else
#  ifndef SCOPY_PKG_MANAGER_EXPORT
#    ifdef scopy_pkg_manager_EXPORTS
        /* We are building this library */
#      define SCOPY_PKG_MANAGER_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_PKG_MANAGER_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_PKG_MANAGER_NO_EXPORT
#    define SCOPY_PKG_MANAGER_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_PKG_MANAGER_DEPRECATED
#  define SCOPY_PKG_MANAGER_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_PKG_MANAGER_DEPRECATED_EXPORT
#  define SCOPY_PKG_MANAGER_DEPRECATED_EXPORT SCOPY_PKG_MANAGER_EXPORT SCOPY_PKG_MANAGER_DEPRECATED
#endif

#ifndef SCOPY_PKG_MANAGER_DEPRECATED_NO_EXPORT
#  define SCOPY_PKG_MANAGER_DEPRECATED_NO_EXPORT SCOPY_PKG_MANAGER_NO_EXPORT SCOPY_PKG_MANAGER_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_PKG_MANAGER_NO_DEPRECATED
#    define SCOPY_PKG_MANAGER_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_PKG_MANAGER_EXPORT_H */
