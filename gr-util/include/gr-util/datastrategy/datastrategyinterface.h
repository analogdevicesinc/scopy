#ifndef SCOPY_DATASTRATEGYINTERFACE_H
#define SCOPY_DATASTRATEGYINTERFACE_H

#include <QObject>
#include <QLoggingCategory>
#include <iio.h>
#include "attrdata.h"

namespace scopy::attr {
class DataStrategyInterface : public QObject
{
	Q_OBJECT
public Q_SLOTS:
	virtual void save(QString data) = 0;
	virtual void requestData() = 0;

Q_SIGNALS:
	void sendData(QString data, QString dataOptions);

protected:
	AttributeFactoryRecipe m_recipe;
};
} // namespace scopy::attr

#endif // SCOPY_DATASTRATEGYINTERFACE_H
