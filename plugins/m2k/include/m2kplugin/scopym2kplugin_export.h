
#ifndef SCOPYM2KPLUGIN_EXPORT_H
#define SCOPYM2KPLUGIN_EXPORT_H

#ifdef SCOPYM2KPLUGIN_STATIC_DEFINE
#  define SCOPYM2KPLUGIN_EXPORT
#  define SCOPYM2KPLUGIN_NO_EXPORT
#else
#  ifndef SCOPYM2KPLUGIN_EXPORT
#    ifdef scopym2kplugin_EXPORTS
        /* We are building this library */
#      define SCOPYM2KPLUGIN_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYM2KPLUGIN_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYM2KPLUGIN_NO_EXPORT
#    define SCOPYM2KPLUGIN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYM2KPLUGIN_DEPRECATED
#  define SCOPYM2KPLUGIN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYM2KPLUGIN_DEPRECATED_EXPORT
#  define SCOPYM2KPLUGIN_DEPRECATED_EXPORT SCOPYM2KPLUGIN_EXPORT SCOPYM2KPLUGIN_DEPRECATED
#endif

#ifndef SCOPYM2KPLUGIN_DEPRECATED_NO_EXPORT
#  define SCOPYM2KPLUGIN_DEPRECATED_NO_EXPORT SCOPYM2KPLUGIN_NO_EXPORT SCOPYM2KPLUGIN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYM2KPLUGIN_NO_DEPRECATED
#    define SCOPYM2KPLUGIN_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYM2KPLUGIN_EXPORT_H */
