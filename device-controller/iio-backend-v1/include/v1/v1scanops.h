#pragma once

#include "iioutil/iscanops.h"

namespace scopy::iio {

class V1ScanOps : public IScanOps {
public:
	QVector<ScanResult> scan(const QString &backends = QString()) override;
};

} // namespace scopy::iio
