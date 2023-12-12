#ifndef SCOPY_DATASTRATEGYINTERFACE_H
#define SCOPY_DATASTRATEGYINTERFACE_H

#include <QObject>
#include <QLoggingCategory>
#include <iio.h>
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy::attr {
class SCOPY_IIO_WIDGETS_EXPORT DataStrategyInterface : public QObject
{
	Q_OBJECT
public Q_SLOTS:
	virtual void save(QString data) = 0;
	virtual void requestData() = 0;

Q_SIGNALS:
	void sendData(QString data, QString dataOptions);
	void emitStatus(int status);

protected:
	IIOWidgetFactoryRecipe m_recipe;
};
} // namespace scopy::attr

#endif // SCOPY_DATASTRATEGYINTERFACE_H
