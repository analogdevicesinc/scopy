
#ifndef SCOPYSIGROK_GUI_EXPORT_H
#define SCOPYSIGROK_GUI_EXPORT_H

#ifdef SCOPYSIGROK_GUI_STATIC_DEFINE
#  define SCOPYSIGROK_GUI_EXPORT
#  define SCOPYSIGROK_GUI_NO_EXPORT
#else
#  ifndef SCOPYSIGROK_GUI_EXPORT
#    ifdef scopysigrok_gui_EXPORTS
        /* We are building this library */
#      define SCOPYSIGROK_GUI_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYSIGROK_GUI_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYSIGROK_GUI_NO_EXPORT
#    define SCOPYSIGROK_GUI_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYSIGROK_GUI_DEPRECATED
#  define SCOPYSIGROK_GUI_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYSIGROK_GUI_DEPRECATED_EXPORT
#  define SCOPYSIGROK_GUI_DEPRECATED_EXPORT SCOPYSIGROK_GUI_EXPORT SCOPYSIGROK_GUI_DEPRECATED
#endif

#ifndef SCOPYSIGROK_GUI_DEPRECATED_NO_EXPORT
#  define SCOPYSIGROK_GUI_DEPRECATED_NO_EXPORT SCOPYSIGROK_GUI_NO_EXPORT SCOPYSIGROK_GUI_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYSIGROK_GUI_NO_DEPRECATED
#    define SCOPYSIGROK_GUI_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYSIGROK_GUI_EXPORT_H */
