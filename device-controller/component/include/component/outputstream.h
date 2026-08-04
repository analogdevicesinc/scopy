#pragma once

#include "component/stream.h"
#include "component/streamformat.h"

#include <QUuid>

namespace scopy::component {

class OutputStream : public Stream
{
	Q_OBJECT
public:
	explicit OutputStream(QObject *parent = nullptr)
		: Stream(parent)
	{
	}

	// Writable view of the internal buffer; valid after open(), up to push().
	virtual StreamFormat &writeFormat() = 0;

	Q_INVOKABLE virtual QCoro::Task<CommandResponse<void>> pushAsync() = 0;

Q_SIGNALS:
	void pushSucceeded();
	void pushFailed(const scopy::Error &error);
};

} // namespace scopy::component
