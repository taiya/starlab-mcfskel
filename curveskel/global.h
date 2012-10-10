#pragma once
#include <QtCore/qglobal.h>

#if defined(EXPORTFLAG)
#  define EXPORT Q_DECL_EXPORT
#else
#  define EXPORT Q_DECL_IMPORT
#endif
