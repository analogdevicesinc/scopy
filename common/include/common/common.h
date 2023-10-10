#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace Qt {
static auto endl = ::endl;
static auto SkipEmptyParts = QString::SkipEmptyParts;
} // namespace Qt
#endif

#endif // COMMON_H
