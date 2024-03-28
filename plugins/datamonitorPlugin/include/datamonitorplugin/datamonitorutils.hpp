#ifndef DATAMONITORUTILS_HPP
#define DATAMONITORUTILS_HPP

#include <QObject>
#include "scopy-datamonitorplugin_export.h"

namespace scopy {
namespace datamonitor {
class SCOPY_DATAMONITORPLUGIN_EXPORT DataMonitorUtils : public QObject
{
	Q_OBJECT
public:
	explicit DataMonitorUtils(QObject *parent = nullptr);

	static double getAxisDefaultMinValue();
	static double getAxisDefaultMaxValue();
	static double getXAxisDefaultViewPortSize();
	static QString getToolSettingsId();
	static double getReadIntervalDefaul();
	static QString getPlotDateTimeFormat();
	static int getDefaultPrecision();
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORUTILS_HPP
