#pragma once
#include <QtCore/qglobal.h>

#if defined(DYNAMIC_SURFACEMESH)
#  define DYNAMIC_SURFACEMESH_EXPORT Q_DECL_EXPORT
#else
#  define DYNAMIC_SURFACEMESH_EXPORT Q_DECL_IMPORT
#endif
