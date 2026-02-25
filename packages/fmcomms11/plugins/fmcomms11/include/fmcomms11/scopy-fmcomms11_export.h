
#ifndef SCOPY_FMCOMMS11_EXPORT_H
#define SCOPY_FMCOMMS11_EXPORT_H

#ifdef SCOPY_FMCOMMS11_STATIC_DEFINE
#  define SCOPY_FMCOMMS11_EXPORT
#  define SCOPY_FMCOMMS11_NO_EXPORT
#else
#  ifndef SCOPY_FMCOMMS11_EXPORT
#    ifdef scopy_fmcomms11_EXPORTS
        /* We are building this library */
#      define SCOPY_FMCOMMS11_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_FMCOMMS11_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_FMCOMMS11_NO_EXPORT
#    define SCOPY_FMCOMMS11_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_FMCOMMS11_DEPRECATED
#  define SCOPY_FMCOMMS11_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_FMCOMMS11_DEPRECATED_EXPORT
#  define SCOPY_FMCOMMS11_DEPRECATED_EXPORT SCOPY_FMCOMMS11_EXPORT SCOPY_FMCOMMS11_DEPRECATED
#endif

#ifndef SCOPY_FMCOMMS11_DEPRECATED_NO_EXPORT
#  define SCOPY_FMCOMMS11_DEPRECATED_NO_EXPORT SCOPY_FMCOMMS11_NO_EXPORT SCOPY_FMCOMMS11_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_FMCOMMS11_NO_DEPRECATED
#    define SCOPY_FMCOMMS11_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_FMCOMMS11_EXPORT_H */
