#ifndef MONITORSELECTIONMENU_HPP
#define MONITORSELECTIONMENU_HPP

#include "datamonitormodel.hpp"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QWidget>
#include <menucollapsesection.h>

namespace scopy {
namespace datamonitor {
class MonitorSelectionMenu : public QWidget
{
	Q_OBJECT
public:
	explicit MonitorSelectionMenu(QMap<QString, DataMonitorModel *> *monitorList, QWidget *parent = nullptr);

Q_SIGNALS:
	void monitorToggled(bool toggled, QString monitorName);
	void removeMonitor();

private:
	QVBoxLayout *layout;
	QButtonGroup *monitorsGroup;
	QMap<QString, MenuCollapseSection *> deviceMap;

	void generateDeviceSection(QString device);
	void addMonitor(DataMonitorModel *monitor);
};
} // namespace datamonitor
} // namespace scopy
#endif // MONITORSELECTIONMENU_HPP
