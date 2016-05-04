#pragma once

#include <QtGlobal>

#if defined(QTCUTILITIES_LIBRARY)
#  define QTCUTILITIESSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QTCUTILITIESSHARED_EXPORT Q_DECL_IMPORT
#endif
