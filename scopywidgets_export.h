
#ifndef SCOPYGUI_EXPORT_H
#define SCOPYGUI_EXPORT_H

#ifdef SCOPYWIDGETS_STATIC_DEFINE
#  define SCOPYGUI_EXPORT
#  define SCOPYWIDGETS_NO_EXPORT
#else
#  ifndef SCOPYGUI_EXPORT
#    ifdef ScopyWidgets_EXPORTS
        /* We are building this library */
#      define SCOPYGUI_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYGUI_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYWIDGETS_NO_EXPORT
#    define SCOPYWIDGETS_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYWIDGETS_DEPRECATED
#  define SCOPYWIDGETS_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYWIDGETS_DEPRECATED_EXPORT
#  define SCOPYWIDGETS_DEPRECATED_EXPORT SCOPYGUI_EXPORT SCOPYWIDGETS_DEPRECATED
#endif

#ifndef SCOPYWIDGETS_DEPRECATED_NO_EXPORT
#  define SCOPYWIDGETS_DEPRECATED_NO_EXPORT SCOPYWIDGETS_NO_EXPORT SCOPYWIDGETS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYWIDGETS_NO_DEPRECATED
#    define SCOPYWIDGETS_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYGUI_EXPORT_H */
