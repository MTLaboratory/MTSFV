
#ifndef QUICKER_SFV_PLUGIN_RAR_EXPORT_H
#define QUICKER_SFV_PLUGIN_RAR_EXPORT_H

#ifdef QUICKER_SFV_PLUGIN_RAR_STATIC_DEFINE
#  define QUICKER_SFV_PLUGIN_RAR_EXPORT
#  define QUICKER_SFV_PLUGIN_RAR_NO_EXPORT
#else
#  ifndef QUICKER_SFV_PLUGIN_RAR_EXPORT
#    ifdef quicker_sfv_rar_EXPORTS
        /* We are building this library */
#      define QUICKER_SFV_PLUGIN_RAR_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define QUICKER_SFV_PLUGIN_RAR_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef QUICKER_SFV_PLUGIN_RAR_NO_EXPORT
#    define QUICKER_SFV_PLUGIN_RAR_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef QUICKER_SFV_PLUGIN_RAR_DEPRECATED
#  define QUICKER_SFV_PLUGIN_RAR_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef QUICKER_SFV_PLUGIN_RAR_DEPRECATED_EXPORT
#  define QUICKER_SFV_PLUGIN_RAR_DEPRECATED_EXPORT QUICKER_SFV_PLUGIN_RAR_EXPORT QUICKER_SFV_PLUGIN_RAR_DEPRECATED
#endif

#ifndef QUICKER_SFV_PLUGIN_RAR_DEPRECATED_NO_EXPORT
#  define QUICKER_SFV_PLUGIN_RAR_DEPRECATED_NO_EXPORT QUICKER_SFV_PLUGIN_RAR_NO_EXPORT QUICKER_SFV_PLUGIN_RAR_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef QUICKER_SFV_PLUGIN_RAR_NO_DEPRECATED
#    define QUICKER_SFV_PLUGIN_RAR_NO_DEPRECATED
#  endif
#endif

#endif /* QUICKER_SFV_PLUGIN_RAR_EXPORT_H */
