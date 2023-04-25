
#ifndef SCOPYGR_GUI_EXPORT_H
#define SCOPYGR_GUI_EXPORT_H

#ifdef SCOPYGR_GUI_STATIC_DEFINE
#  define SCOPYGR_GUI_EXPORT
#  define SCOPYGR_GUI_NO_EXPORT
#else
#  ifndef SCOPYGR_GUI_EXPORT
#    ifdef scopygr_gui_EXPORTS
        /* We are building this library */
#      define SCOPYGR_GUI_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYGR_GUI_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYGR_GUI_NO_EXPORT
#    define SCOPYGR_GUI_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYGR_GUI_DEPRECATED
#  define SCOPYGR_GUI_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYGR_GUI_DEPRECATED_EXPORT
#  define SCOPYGR_GUI_DEPRECATED_EXPORT SCOPYGR_GUI_EXPORT SCOPYGR_GUI_DEPRECATED
#endif

#ifndef SCOPYGR_GUI_DEPRECATED_NO_EXPORT
#  define SCOPYGR_GUI_DEPRECATED_NO_EXPORT SCOPYGR_GUI_NO_EXPORT SCOPYGR_GUI_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYGR_GUI_NO_DEPRECATED
#    define SCOPYGR_GUI_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYGR_GUI_EXPORT_H */
