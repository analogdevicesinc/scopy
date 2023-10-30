#include "triggerhandlermock.h"
#include <QObject>

using namespace scopy;

TriggerHandlerMock::TriggerHandlerMock(QObject *parent) { setParent(parent); }

QList<QString> TriggerHandlerMock::getAvailableTriggerNames() { return m_triggerList; }

void TriggerHandlerMock::setTrigger(QString deviceName, QString triggerName)
{
	m_triggerConnections.insert(deviceName, triggerName);
}

void TriggerHandlerMock::removeTrigger(QString deviceName)
{
	if(m_triggerConnections.contains(deviceName)) {
		m_triggerConnections.remove(deviceName);
	}
}

QString TriggerHandlerMock::getTriggerFromDevice(QString device) { return m_triggerConnections[device]; }

struct iio_device *TriggerHandlerMock::getTriggerWithName(QString trigger) { return nullptr; }

#include "moc_triggerhandlermock.cpp"
