#ifndef MONITORSELECTIONMENU_HPP
#define MONITORSELECTIONMENU_HPP

#include "datamonitor/datamonitormodel.hpp"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QWidget>
#include <menucollapsesection.h>
#include <semiexclusivebuttongroup.h>

namespace scopy {
namespace datamonitor {
class MonitorSelectionMenu : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit MonitorSelectionMenu(QMap<QString, DataMonitorModel *> *monitorList, QButtonGroup *monitorsGroup,
				      QWidget *parent = nullptr);

	void addMonitor(DataMonitorModel *monitor);
	void removeDevice(QString device);

	QButtonGroup *monitorsGroup() const;

Q_SIGNALS:
	void requestMonitorToggled(bool toggled, QString monitorName);
	void monitorToggled(bool toggled, QString monitorName);
	void removeMonitor();
	void requestRemoveImportedDevice(QString device);
	void requestMonitorMenu(bool toggled, QString monitorName);

private:
	QWidget *deviceChannelsWidget;
	QWidget *importedChannelsWidget;
	QVBoxLayout *layout;
	QButtonGroup *m_monitorsGroup;
	QMap<QString, MenuCollapseSection *> deviceMap;

	void generateDeviceSection(QString device, bool import = false);
};
} // namespace datamonitor
} // namespace scopy
#endif // MONITORSELECTIONMENU_HPP