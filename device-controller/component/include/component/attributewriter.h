#pragma once

#include "core/result.h"

#include <QObject>
#include <QString>
#include <QUuid>
#include <qcorotask.h>

namespace scopy::component {

// Write capability (abstract base). Child of an Attribute; writeSucceeded is
// wired to the reader's readAsync() for the write-then-read-back settle.
class AttributeWriter : public QObject
{
	Q_OBJECT
public:
	explicit AttributeWriter(QObject *parent = nullptr)
		: QObject(parent)
	{
	}
	~AttributeWriter() override = default;

    Q_INVOKABLE virtual QCoro::Task<CommandResponse<void>> writeAsync(const QString &value) = 0;

Q_SIGNALS:
	void writeSucceeded();
	void writeFailed(const scopy::Error &error);
};

} // namespace scopy::component
