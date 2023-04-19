
#ifndef SCOPYTESTPLUGIN_EXPORT_H
#define SCOPYTESTPLUGIN_EXPORT_H

#ifdef SCOPYTESTPLUGIN_STATIC_DEFINE
#  define SCOPYTESTPLUGIN_EXPORT
#  define SCOPYTESTPLUGIN_NO_EXPORT
#else
#  ifndef SCOPYTESTPLUGIN_EXPORT
#    ifdef scopytestplugin_EXPORTS
        /* We are building this library */
#      define SCOPYTESTPLUGIN_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYTESTPLUGIN_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYTESTPLUGIN_NO_EXPORT
#    define SCOPYTESTPLUGIN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYTESTPLUGIN_DEPRECATED
#  define SCOPYTESTPLUGIN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYTESTPLUGIN_DEPRECATED_EXPORT
#  define SCOPYTESTPLUGIN_DEPRECATED_EXPORT SCOPYTESTPLUGIN_EXPORT SCOPYTESTPLUGIN_DEPRECATED
#endif

#ifndef SCOPYTESTPLUGIN_DEPRECATED_NO_EXPORT
#  define SCOPYTESTPLUGIN_DEPRECATED_NO_EXPORT SCOPYTESTPLUGIN_NO_EXPORT SCOPYTESTPLUGIN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYTESTPLUGIN_NO_DEPRECATED
#    define SCOPYTESTPLUGIN_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYTESTPLUGIN_EXPORT_H */
