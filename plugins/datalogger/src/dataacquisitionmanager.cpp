#include "dataacquisitionmanager.hpp"

#include <datamonitor/readabledatamonitormodel.hpp>

using namespace scopy;
using namespace datamonitor;

DataAcquisitionManager::DataAcquisitionManager(QObject *parent)
	: QObject{parent}
{
	m_activeMonitorsMap = new QMap<QString, int>();
	m_dataMonitorMap = new QMap<QString, DataMonitorModel *>();
}

void DataAcquisitionManager::addMonitor(DataMonitorModel *monitor)
{
	getDataMonitorMap()->insert(monitor->getName(), monitor);
	Q_EMIT monitorAdded(monitor);
}

void DataAcquisitionManager::removeMonitor(QString monitorName)
{
	if(m_activeMonitorsMap->contains(monitorName)) {
		m_activeMonitorsMap->remove(monitorName);
	}
	if(getDataMonitorMap()->contains(monitorName)) {
		delete getDataMonitorMap()->value(monitorName);
		getDataMonitorMap()->remove(monitorName);
		Q_EMIT monitorRemoved(monitorName);
	}
}

void DataAcquisitionManager::removeDevice(QString device)
{
	foreach(QString monKey, getDataMonitorMap()->keys()) {
		if(m_dataMonitorMap->value(monKey)->getDeviceName() == device) {
			removeMonitor(monKey);
		}
	}

	Q_EMIT deviceRemoved(device);
}

void DataAcquisitionManager::clearMonitorsData()
{
	foreach(QString monKey, m_activeMonitorsMap->keys()) {
		m_dataMonitorMap->value(monKey)->clearMonitorData();
	}
}

void DataAcquisitionManager::readData()
{
	// TODO add a readthread
	//		readerThread->start();

	foreach(QString monKey, m_activeMonitorsMap->keys()) {
		auto monitor = m_dataMonitorMap->value(monKey);
		if(qobject_cast<ReadableDataMonitorModel *>(monitor)) {
			dynamic_cast<ReadableDataMonitorModel *>(monitor)->read();
		}
	}
}

void DataAcquisitionManager::updateActiveMonitors(bool toggled, QString monitorName)
{
	if(toggled) {
		// monitor enabled
		if(m_activeMonitorsMap->contains(monitorName)) {
			// more than one view is using the monitor
			int value = m_activeMonitorsMap->value(monitorName);
			m_activeMonitorsMap->insert(monitorName, ++value);

		} else {
			// one view is using the monitor
			m_activeMonitorsMap->insert(monitorName, 1);
		}
	} else {
		// monitor disabled
		if(m_activeMonitorsMap->value(monitorName) <= 1) {
			// when no view has the monitor enabled remove it from active monitors
			m_activeMonitorsMap->remove(monitorName);
		} else {
			// one less view is using the monitor
			int value = m_activeMonitorsMap->value(monitorName);
			m_activeMonitorsMap->insert(monitorName, --value);
		}
	}
	Q_EMIT activeMonitorsUpdated();
}

QList<QString> DataAcquisitionManager::getActiveMonitors() { return m_activeMonitorsMap->keys(); }

QMap<QString, DataMonitorModel *> *DataAcquisitionManager::getDataMonitorMap() const { return m_dataMonitorMap; }
