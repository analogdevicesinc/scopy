#ifndef DATAACQUISITIONMANAGER_HPP
#define DATAACQUISITIONMANAGER_HPP

#include <QElapsedTimer>
#include <QMap>
#include <QObject>

#include "datamonitor/datamonitormodel.hpp"
#include "scopy-dataloggerplugin_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGERPLUGIN_EXPORT DataAcquisitionManager : public QObject
{
	Q_OBJECT
public:
	explicit DataAcquisitionManager(QObject *parent = nullptr);

	void addMonitor(DataMonitorModel *monitor);
	void removeMonitor(QString monitorName);
	void removeDevice(QString device);
	void clearMonitorsData();

	// will read data once
	void readData();
	QList<QString> getActiveMonitors();
	void updateActiveMonitors(bool toggled, QString monitorName);
	QMap<QString, DataMonitorModel *> *getDataMonitorMap() const;

Q_SIGNALS:
	void requestUpdateActiveMonitors();
	void requestRead();
	void activeMonitorsUpdated();
	void monitorAdded(DataMonitorModel *monitor);
	void monitorRemoved(QString monitorName);
	void deviceRemoved(QString deviceName);

private:
	QMap<QString, int> *m_activeMonitorsMap;
	QMap<QString, DataMonitorModel *> *m_dataMonitorMap;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAACQUISITIONMANAGER_HPP
