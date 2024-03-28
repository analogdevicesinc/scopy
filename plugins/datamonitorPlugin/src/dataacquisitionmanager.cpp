#include "dataacquisitionmanager.hpp"

using namespace scopy;
using namespace datamonitor;

DataAcquisitionManager::DataAcquisitionManager(QObject *parent)
	: QObject{parent}
{
	m_activeMonitorsMap = new QMap<QString, int>();
	m_dataMonitorMap = new QMap<QString, DataMonitorModel *>();
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
		m_dataMonitorMap->value(monKey)->read();
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
