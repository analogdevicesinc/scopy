
#ifndef SCOPYGR_SCOPYGUI_EXPORT_H
#define SCOPYGR_SCOPYGUI_EXPORT_H

#ifdef SCOPYGR_SCOPYGUI_STATIC_DEFINE
#  define SCOPYGR_SCOPYGUI_EXPORT
#  define SCOPYGR_SCOPYGUI_NO_EXPORT
#else
#  ifndef SCOPYGR_SCOPYGUI_EXPORT
#    ifdef scopygr_scopygui_EXPORTS
        /* We are building this library */
#      define SCOPYGR_SCOPYGUI_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYGR_SCOPYGUI_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYGR_SCOPYGUI_NO_EXPORT
#    define SCOPYGR_SCOPYGUI_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYGR_SCOPYGUI_DEPRECATED
#  define SCOPYGR_SCOPYGUI_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYGR_SCOPYGUI_DEPRECATED_EXPORT
#  define SCOPYGR_SCOPYGUI_DEPRECATED_EXPORT SCOPYGR_SCOPYGUI_EXPORT SCOPYGR_SCOPYGUI_DEPRECATED
#endif

#ifndef SCOPYGR_SCOPYGUI_DEPRECATED_NO_EXPORT
#  define SCOPYGR_SCOPYGUI_DEPRECATED_NO_EXPORT SCOPYGR_SCOPYGUI_NO_EXPORT SCOPYGR_SCOPYGUI_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYGR_SCOPYGUI_NO_DEPRECATED
#    define SCOPYGR_SCOPYGUI_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYGR_SCOPYGUI_EXPORT_H */
