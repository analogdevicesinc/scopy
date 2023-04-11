
#ifndef SCOPYDEBUGGER_EXPORT_H
#define SCOPYDEBUGGER_EXPORT_H

#ifdef SCOPYDEBUGGER_STATIC_DEFINE
#  define SCOPYDEBUGGER_EXPORT
#  define SCOPYDEBUGGER_NO_EXPORT
#else
#  ifndef SCOPYDEBUGGER_EXPORT
#    ifdef scopydebugger_EXPORTS
        /* We are building this library */
#      define SCOPYDEBUGGER_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SCOPYDEBUGGER_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SCOPYDEBUGGER_NO_EXPORT
#    define SCOPYDEBUGGER_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SCOPYDEBUGGER_DEPRECATED
#  define SCOPYDEBUGGER_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SCOPYDEBUGGER_DEPRECATED_EXPORT
#  define SCOPYDEBUGGER_DEPRECATED_EXPORT SCOPYDEBUGGER_EXPORT SCOPYDEBUGGER_DEPRECATED
#endif

#ifndef SCOPYDEBUGGER_DEPRECATED_NO_EXPORT
#  define SCOPYDEBUGGER_DEPRECATED_NO_EXPORT SCOPYDEBUGGER_NO_EXPORT SCOPYDEBUGGER_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SCOPYDEBUGGER_NO_DEPRECATED
#    define SCOPYDEBUGGER_NO_DEPRECATED
#  endif
#endif

#endif /* SCOPYDEBUGGER_EXPORT_H */
