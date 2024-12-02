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
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit MonitorSelectionMenu(QMap<QString, DataMonitorModel *> *monitorList, QWidget *parent = nullptr);

	void addMonitor(DataMonitorModel *monitor);
	void removeDevice(QString device);

Q_SIGNALS:
	void monitorToggled(bool toggled, QString monitorName);
	void removeMonitor();
	void requestRemoveImportedDevice(QString device);
	void requestMonitorMenu(QString monitorName);

private:
	QWidget *deviceChannelsWidget;
	QWidget *importedChannelsWidget;
	QVBoxLayout *layout;
	QButtonGroup *monitorsGroup;
	QMap<QString, MenuCollapseSection *> deviceMap;

	void generateDeviceSection(QString device, bool import = false);
};
} // namespace datamonitor
} // namespace scopy
#endif // MONITORSELECTIONMENU_HPP
