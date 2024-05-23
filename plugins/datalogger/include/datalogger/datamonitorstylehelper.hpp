#ifndef DATAMONITORSTYLEHELPER_HPP
#define DATAMONITORSTYLEHELPER_HPP

#include <QObject>
#include "menus/datamonitorsettings.hpp"
#include "datamonitortool.h"
#include "datamonitor/sevensegmentmonitor.hpp"
#include "menus/sevensegmentmonitorsettings.hpp"

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
	static void DataLoggingMenuStyle(DataLoggingMenu *menu);
	static void MonitorSelectionMenuMenuCollapseSectionStyle(MenuCollapseSection *menu);
	static QString RemoveButtonStyle();

signals:
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORSTYLEHELPER_HPP
