#ifndef SCOPY_DATASTRATEGYINTERFACE_H
#define SCOPY_DATASTRATEGYINTERFACE_H

#include <QObject>
#include <QLoggingCategory>
#include <iio.h>
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT DataStrategyInterface
{
public:
	virtual ~DataStrategyInterface() = default;

public Q_SLOTS:
	virtual void save(QString data) = 0;
	virtual void requestData() = 0;

Q_SIGNALS:
	virtual void sendData(QString data, QString dataOptions) = 0;
	virtual void emitStatus(int status) = 0;

protected:
	IIOWidgetFactoryRecipe m_recipe;
};
} // namespace scopy

Q_DECLARE_INTERFACE(scopy::DataStrategyInterface, "scopy::DataStrategyInterface")
#endif // SCOPY_DATASTRATEGYINTERFACE_H
