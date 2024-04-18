#ifndef DMM_HPP
#define DMM_HPP

#include <QList>
#include <QMap>
#include <QObject>
#include "dmmdatamonitormodel.hpp"
#include "iio.h"
#include "scopy-datamonitorplugin_export.h"

namespace scopy {
namespace datamonitor {

struct DMMInfo
{
	QString key;
	QString key_symbol;
	double umScale = 1;
};

class SCOPY_DATAMONITORPLUGIN_EXPORT DMM : public QObject
{
	Q_OBJECT
public:
	explicit DMM(QObject *parent = nullptr);

	QList<DmmDataMonitorModel *> getDmmMonitors(iio_context *ctx);
	bool isDMMCompatible(iio_channel *chn);
	bool isHwmon(iio_device *dev, iio_channel *chn);
	bool iioChannelHasAttribute(iio_channel *chn, std::string const &attr);
	void generateDictionaries();

	QMap<int, DMMInfo> iioDevices() const;
	QMap<int, DMMInfo> hwmonDevices() const;

private:
	QMap<int, DMMInfo> m_iioDevices;
	QMap<int, DMMInfo> m_hwmonDevices;
};
} // namespace datamonitor
} // namespace scopy
#endif // DMM_HPP
