#ifndef DMM_HPP
#define DMM_HPP

#include <QList>
#include <QMap>
#include <QObject>
#include "datamonitor/dmmdatamonitormodel.hpp"
#include "iio.h"
#include "scopy-datalogger_export.h"
#include "iioutil/iiounits.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT DMM : public QObject
{
	Q_OBJECT
public:
	explicit DMM(QObject *parent = nullptr);

	QList<DmmDataMonitorModel *> getDmmMonitors(iio_context *ctx);
	bool isDMMCompatible(iio_channel *chn);
	bool isHwmon(iio_device *dev, iio_channel *chn);
	bool iioChannelHasAttribute(iio_channel *chn, std::string const &attr);
	void generateDictionaries();

	QMap<iio_chan_type, IIOUnit> iioDevices() const;
	QMap<hwmon_chan_type, IIOUnit> hwmonDevices() const;

private:
	QMap<iio_chan_type, IIOUnit> m_iioDevices;
	QMap<hwmon_chan_type, IIOUnit> m_hwmonDevices;
};
} // namespace datamonitor
} // namespace scopy
#endif // DMM_HPP
