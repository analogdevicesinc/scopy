#pragma once

#include <qcoro/qcorotask.h>

namespace scopy::iio {

class BufferUser
{
public:
	virtual ~BufferUser() = default;
	virtual QCoro::Task<bool> stop() = 0;
};

} // namespace scopy::iio
