#include "triggerhandlerimpl.h"
#include <QLoggingCategory>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_TRIGGER_HANDLER, "TriggerHandler")

TriggerHandlerImpl::TriggerHandlerImpl(struct iio_context *context, QObject *parent)
	: m_context(context)
{
	setParent(parent);
	init();
	applyMatchingTriggers();
}

TriggerHandlerImpl::~TriggerHandlerImpl() {}

QList<QString> TriggerHandlerImpl::getAvailableTriggerNames() { return m_triggers.keys(); }

QString TriggerHandlerImpl::getTriggerFromDevice(QString device) { return m_connections[device]; }

struct iio_device *TriggerHandlerImpl::getTriggerWithName(QString trigger) { return m_triggers[trigger]; }

void TriggerHandlerImpl::setTrigger(QString deviceName, QString triggerName)
{
	if(triggerName == NO_TRIGGER_NAME) {
		removeTrigger(deviceName);
		return;
	}

	if(!m_connections.contains(deviceName)) {
		qCritical(CAT_TRIGGER_HANDLER)
			<< "Cannot set trigger, device" << deviceName << "cannot be found in connections.";
		return;
	}

	if(!m_triggers.contains(triggerName)) {
		qCritical(CAT_TRIGGER_HANDLER)
			<< "Cannot set trigger, trigger" << triggerName << "cannot be found in connections.";
		return;
	}

	if(m_connections[deviceName] == triggerName) {
		qInfo(CAT_TRIGGER_HANDLER)
			<< "The device" << deviceName << "seems to already have the trigger" << triggerName << "set.";
		return;
	}

	struct iio_device *device = iio_context_find_device(m_context, deviceName.toStdString().c_str());
	if(!device) {
		qWarning(CAT_TRIGGER_HANDLER)
			<< "Could not find the selected trigger" << triggerName << "in the context";
	}

	bool isTrigger = iio_device_is_trigger(device);
	if(isTrigger) {
		qWarning(CAT_TRIGGER_HANDLER)
			<< "Cannot set trigger" << triggerName << "to another trigger" << deviceName;
		return;
	}

	struct iio_device *trigger = m_triggers[triggerName];
	const struct iio_device *triggerReadback;
	int res = iio_device_set_trigger(device, trigger);
	int readbackResult = iio_device_get_trigger(device, &triggerReadback);

	if(trigger == triggerReadback) {
		qInfo(CAT_TRIGGER_HANDLER) << "Trigger" << triggerName << "is successfully set on device" << deviceName;
		m_connections[deviceName] = triggerName;
	}

	if(res != 0) {
		qWarning(CAT_TRIGGER_HANDLER) << "The trigger set function returned error code" << res;
	}

	if(readbackResult != 0) {
		qWarning(CAT_TRIGGER_HANDLER) << "The trigger read function returned error code" << readbackResult;
	}
}

void TriggerHandlerImpl::removeTrigger(QString deviceName)
{
	if(!m_connections.contains(deviceName)) {
		qWarning(CAT_TRIGGER_HANDLER) << "Cannot remove trigger from device" << deviceName
					      << "as the device cannot be found in the list";
		return;
	}

	struct iio_device *device = iio_context_find_device(m_context, deviceName.toStdString().c_str());
	if(device) {
		int res = iio_device_set_trigger(device, nullptr);
		const struct iio_device *trigger;
		int readTriggerRes = iio_device_get_trigger(device, &trigger);
		if(readTriggerRes != 0) {
			qWarning(CAT_TRIGGER_HANDLER)
				<< "Trigger readback returned" << readTriggerRes << "for device" << deviceName;
		}

		if(trigger == nullptr) {
			qInfo(CAT_TRIGGER_HANDLER) << "Trigger from" << deviceName << "device successfully removed";
		}

		if(res != 0) {
			qWarning(CAT_TRIGGER_HANDLER) << "Cannot remove trigger from device" << deviceName;
		}
	} else {
		qWarning(CAT_TRIGGER_HANDLER) << "Cannot find device with name" << deviceName;
	}
}

void TriggerHandlerImpl::init()
{
	unsigned int deviceCount = iio_context_get_devices_count(m_context);
	for(int i = 0; i < deviceCount; ++i) {
		struct iio_device *device = iio_context_get_device(m_context, i);
		if(!device) {
			qWarning(CAT_TRIGGER_HANDLER)
				<< "There is no device with index" << i << "in the current context";
			return;
		}

		QString name = iio_device_get_name(device); // if the device has no name, NULL is returned.

		bool isTrigger = iio_device_is_trigger(device);
		if(isTrigger) {
			m_triggers.insert(name, device);
			continue;
		}

		const struct iio_device *trigger;
		int res = iio_device_get_trigger(device, &trigger);
		if(res) {
			qWarning(CAT_TRIGGER_HANDLER) << "The trigger read function returned error code" << res;
		}

		QString triggerName = NO_TRIGGER_NAME;
		if(trigger) {
			triggerName = iio_device_get_name(trigger);
		}

		m_connections.insert(name, triggerName);
	}
}

QString TriggerHandlerImpl::getMatchingTriggerForDevice(const QString &deviceName)
{
	QList<QString> triggerNames = m_triggers.keys();
	for(QString trigger : triggerNames) {
		if(trigger.startsWith(deviceName)) {
			return trigger;
		}
	}

	return "";
}

void TriggerHandlerImpl::applyMatchingTriggers()
{
	QList<QString> deviceNames = m_connections.keys();
	for(const QString &deviceName : deviceNames) {
		if(m_connections[deviceName] != NO_TRIGGER_NAME) {
			continue;
		}
		QString matchingTrigger = getMatchingTriggerForDevice(deviceName);
		if(matchingTrigger != "") {
			qInfo(CAT_TRIGGER_HANDLER)
				<< "Found possible trigger \"" << matchingTrigger << "\" for device" << deviceName;
			setTrigger(deviceName, matchingTrigger);
		}
	}
}

#include "moc_triggerhandlerimpl.cpp"
