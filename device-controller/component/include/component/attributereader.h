#pragma once

#include "core/result.h"

#include <QObject>
#include <QUuid>
#include <qcorotask.h>

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

    Q_INVOKABLE virtual QCoro::Task<CommandResponse<QByteArray>> readAsync() = 0;

Q_SIGNALS:
    void readSucceeded(scopy::Result<QByteArray> &result);
	void readFailed(const scopy::Error &error);
};

} // namespace scopy::component

Q_DECLARE_METATYPE(scopy::Result<QByteArray>)
