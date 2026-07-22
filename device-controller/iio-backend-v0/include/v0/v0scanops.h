#pragma once

#include "iioutil/iscanops.h"

namespace scopy::iio {

class V0ScanOps : public IScanOps
{
public:
	QVector<ScanResult> scan(const QString &backends = QString()) override;
};

} // namespace scopy::iio
