#pragma once

#include "core/result.h"

#include <QObject>
#include <QUuid>

namespace scopy::component {

// Read capability (abstract base). Child of an Attribute; readSucceeded is wired
// to Attribute::setCachedValue.
class AttributeReader : public QObject
{
	Q_OBJECT
public:
	explicit AttributeReader(QObject *parent = nullptr)
		: QObject(parent)
	{
	}
	~AttributeReader() override = default;

	virtual Result<QByteArray> read() = 0;
	Q_INVOKABLE virtual QUuid readAsync() = 0; // returns the dispatched command's id

Q_SIGNALS:
	void readSucceeded(Result<QByteArray> &result);
	void readFailed(const scopy::Error &error);
};

} // namespace scopy::component

Q_DECLARE_METATYPE(scopy::Result<QByteArray>)
