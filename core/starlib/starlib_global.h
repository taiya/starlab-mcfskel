#pragma once
#if defined(DYNAMIC_STARLIB)
#  define STARLIB_EXPORT Q_DECL_EXPORT
#else
#  define STARLIB_EXPORT Q_DECL_IMPORT
#endif
