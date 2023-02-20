
#ifndef SCOPY_GUI_EXPORT_H
#define SCOPY_GUI_EXPORT_H

#ifdef SCOPY_GUI_STATIC_DEFINE
#  define SCOPY_GUI_EXPORT
#  define SCOPY_GUI_NO_EXPORT
#else
#  ifndef SCOPY_GUI_EXPORT
#    ifdef scopy_gui_EXPORTS
        /* We are building this library */
#      define SCOPY_GUI_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPY_GUI_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPY_GUI_NO_EXPORT
#    define SCOPY_GUI_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPY_GUI_DEPRECATED
#  define SCOPY_GUI_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPY_GUI_DEPRECATED_EXPORT
#  define SCOPY_GUI_DEPRECATED_EXPORT SCOPY_GUI_EXPORT SCOPY_GUI_DEPRECATED
#endif

#ifndef SCOPY_GUI_DEPRECATED_NO_EXPORT
#  define SCOPY_GUI_DEPRECATED_NO_EXPORT SCOPY_GUI_NO_EXPORT SCOPY_GUI_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPY_GUI_NO_DEPRECATED
#    define SCOPY_GUI_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPY_GUI_EXPORT_H */
