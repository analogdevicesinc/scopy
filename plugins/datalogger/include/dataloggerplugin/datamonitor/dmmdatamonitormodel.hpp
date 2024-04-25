#ifndef DMMDATAMONITORMODEL_HPP
#define DMMDATAMONITORMODEL_HPP

#include "datamonitor/readstrategy/dmmreadstrategy.hpp"
#include "readabledatamonitormodel.hpp"
#include "scopy-dataloggerplugin_export.h"
#include <QObject>

namespace scopy {
namespace datamonitor {
class SCOPY_DATALOGGERPLUGIN_EXPORT DmmDataMonitorModel : public ReadableDataMonitorModel
{
	Q_OBJECT
public:
	DmmDataMonitorModel(QObject *parent = nullptr);
	explicit DmmDataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure = nullptr,
				     DMMReadStrategy *readStrategy = nullptr, QObject *parent = nullptr);

	iio_channel *iioChannel() const;
	void setIioChannel(iio_channel *newIioChannel);

	QString getDeviceName();
	iio_device *iioDevice() const;
	void setIioDevice(iio_device *newIioDevice);

private:
	iio_channel *m_iioChannel;
	iio_device *m_iioDevice;
};
} // namespace datamonitor
} // namespace scopy
#endif // DMMDATAMONITORMODEL_HPP
