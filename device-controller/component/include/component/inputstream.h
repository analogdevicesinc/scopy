#pragma once

#include "component/stream.h"
#include "component/streamformat.h"

#include <QUuid>

namespace scopy::component {

class InputStream : public Stream
{
	Q_OBJECT
public:
	explicit InputStream(QObject *parent = nullptr)
		: Stream(parent)
	{
	}

	virtual Result<void> refill() = 0;
	Q_INVOKABLE virtual QUuid refillAsync() = 0;

	// Read-only view of the captured buffer; valid after a successful refill().
	virtual const StreamFormat &readFormat() const = 0;

Q_SIGNALS:
	void refillSucceeded();
	void refillFailed(const scopy::Error &error);
};

} // namespace scopy::component
