
#ifndef SCOPYGUI_EXPORT_H
#define SCOPYGUI_EXPORT_H

#ifdef SCOPYGUI_STATIC_DEFINE
#  define SCOPYGUI_EXPORT
#  define SCOPYGUI_NO_EXPORT
#else
#  ifndef SCOPYGUI_EXPORT
#    ifdef scopygui_EXPORTS
        /* We are building this library */
#      define SCOPYGUI_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYGUI_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYGUI_NO_EXPORT
#    define SCOPYGUI_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYGUI_DEPRECATED
#  define SCOPYGUI_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYGUI_DEPRECATED_EXPORT
#  define SCOPYGUI_DEPRECATED_EXPORT SCOPYGUI_EXPORT SCOPYGUI_DEPRECATED
#endif

#ifndef SCOPYGUI_DEPRECATED_NO_EXPORT
#  define SCOPYGUI_DEPRECATED_NO_EXPORT SCOPYGUI_NO_EXPORT SCOPYGUI_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYGUI_NO_DEPRECATED
#    define SCOPYGUI_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYGUI_EXPORT_H */
