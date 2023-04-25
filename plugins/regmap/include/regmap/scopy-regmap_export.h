
#ifndef SCOPY_REGMAP_EXPORT_H
#define SCOPY_REGMAP_EXPORT_H

#ifdef SCOPY_REGMAP_STATIC_DEFINE
#  define SCOPY_REGMAP_EXPORT
#  define SCOPY_REGMAP_NO_EXPORT
#else
#  ifndef SCOPY_REGMAP_EXPORT
#    ifdef scopy_regmap_EXPORTS
        /* We are building this library */
#      define SCOPY_REGMAP_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_REGMAP_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_REGMAP_NO_EXPORT
#    define SCOPY_REGMAP_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_REGMAP_DEPRECATED
#  define SCOPY_REGMAP_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_REGMAP_DEPRECATED_EXPORT
#  define SCOPY_REGMAP_DEPRECATED_EXPORT SCOPY_REGMAP_EXPORT SCOPY_REGMAP_DEPRECATED
#endif

#ifndef SCOPY_REGMAP_DEPRECATED_NO_EXPORT
#  define SCOPY_REGMAP_DEPRECATED_NO_EXPORT SCOPY_REGMAP_NO_EXPORT SCOPY_REGMAP_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_REGMAP_NO_DEPRECATED
#    define SCOPY_REGMAP_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_REGMAP_EXPORT_H */
