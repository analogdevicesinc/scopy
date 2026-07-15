#pragma once

#include "iioutil/handles.h"
#include <QString>
#include <QVector>

namespace scopy::iio {

class IScanOps {
public:
	virtual ~IScanOps() = default;
	virtual QVector<ScanResult> scan(const QString &backends = QString()) = 0;
};

} // namespace scopy::iio
