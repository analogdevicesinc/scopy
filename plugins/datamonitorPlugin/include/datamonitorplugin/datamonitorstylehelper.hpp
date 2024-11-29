#ifndef DATAMONITORSTYLEHELPER_HPP
#define DATAMONITORSTYLEHELPER_HPP

#include <QObject>
#include "datamonitorsettings.hpp"
#include "datamonitortool.h"
#include "sevensegmentmonitor.hpp"
#include "sevensegmentmonitorsettings.hpp"

namespace scopy {
namespace datamonitor {

class DataMonitorStyleHelper : public QObject
{
	Q_OBJECT
public:
	explicit DataMonitorStyleHelper(QObject *parent = nullptr);

	static void DataMonitorSettingsStyle(DataMonitorSettings *dataMonitorSettings);
	static void DataMonitorToolStyle(DatamonitorTool *tool);
	static void SevenSegmentMonitorsStyle(SevenSegmentMonitor *sevenSegmentMonitor, QString lcdColor);
	static void SevenSegmentMonitorMenuStyle(SevenSegmentMonitorSettings *sevenSegmentMonitorSettings);
	static QString RemoveButtonStyle();

signals:
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORSTYLEHELPER_HPP
