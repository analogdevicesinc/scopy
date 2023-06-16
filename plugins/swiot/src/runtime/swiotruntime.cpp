#include "swiotruntime.h"
#include <string>
#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;

SwiotRuntime::SwiotRuntime(QObject *parent)
	: QObject(parent)
	, m_iioCtx(nullptr)
	, m_triggerTimer(nullptr)
{}

SwiotRuntime::~SwiotRuntime()
{
	if (m_triggerTimer) {
		m_triggerTimer->stop();
		delete m_triggerTimer;
		m_triggerTimer = nullptr;
	}
}

bool SwiotRuntime::isRuntimeCtx()
{
	if (m_iioDevices.contains(SWIOT_DEVICE_NAME) && m_iioDevices[SWIOT_DEVICE_NAME]) {
		const char* modeAttribute = iio_device_find_attr(m_iioDevices[SWIOT_DEVICE_NAME], "mode");
		if (modeAttribute) {
			char mode[64];
			ssize_t result = iio_device_attr_read(m_iioDevices[SWIOT_DEVICE_NAME], "mode", mode, 64);

			if (result < 0) {
				qCritical(CAT_SWIOT) << R"(Critical error: could not read mode attribute, error code:)" << result;
			}

			if (strcmp(mode, "runtime") == 0) {
				return true;
			} else {
				return false;
			}
		} else {
			qCritical(CAT_SWIOT) << R"(Critical error: did not find "mode" attribute in the "swiot" device)";
		}
	} else {
		qCritical(CAT_SWIOT) << R"(Critical error: did not find "swiot" device in the context)";
	}

	return false;
}

void SwiotRuntime::setContext(iio_context *ctx)
{
	m_iioCtx = ctx;
	createDevicesMap();
	bool isRuntime = isRuntimeCtx();
	if (isRuntime) {
		qInfo(CAT_SWIOT) <<"runtime context";
		if (m_iioDevices.contains(AD_TRIGGER_NAME)) {
			int retCode = iio_device_set_trigger(m_iioDevices[AD_NAME], m_iioDevices[AD_TRIGGER_NAME]);
			qDebug(CAT_SWIOT) <<"Trigger has been set: " + QString::number(retCode);
		}
		//		m_triggerTimer = new QTimer();
		//		m_triggerTimer->start(TRIGGER_TIMER_MS);
		//		connect(m_triggerTimer, SIGNAL(timeout()), this, SLOT(onTriggerTimeout()));
	} else {
		qDebug(CAT_SWIOT) << "isn't runtime context";
	}
}

//void SwiotRuntime::triggerPing()
//{
//	if (m_iioDevices[AD_NAME]) {
//		const iio_device* triggerDevice = nullptr;
//		int ret = iio_device_get_trigger(m_iioDevices[AD_NAME], &triggerDevice);

//		if (ret < 0 && m_iioDevices.contains(AD_TRIGGER_NAME) && m_iioDevices[AD_TRIGGER_NAME]) {
//			iio_device_set_trigger(m_iioDevices[AD_NAME], m_iioDevices[AD_TRIGGER_NAME]);
//			qDebug(CAT_SWIOT_RUNTIME) << "Trigger has been set";
//		} else {
//			qDebug(CAT_SWIOT_RUNTIME) << QString::number(ret) +" returned";
//		}
//	}
//}

//void SwiotRuntime::onTriggerTimeout()
//{
//	QtConcurrent::run(std::mem_fn(&SwiotRuntime::triggerPing), this);
//}

void SwiotRuntime::createDevicesMap()
{
	if (m_iioCtx) {
		int devicesNumber = iio_context_get_devices_count(m_iioCtx);
		m_iioDevices.clear();
		for (int i = 0; i < devicesNumber; i++) {
			struct iio_device* iioDev = iio_context_get_device(m_iioCtx, i);
			if (iioDev) {
				QString deviceName = QString(iio_device_get_name(iioDev));
				m_iioDevices[deviceName] = iioDev;
			}
		}
	}
}

void SwiotRuntime::onBackBtnPressed()
{
	if (m_iioDevices.contains(SWIOT_DEVICE_NAME) && m_iioDevices[SWIOT_DEVICE_NAME]) {
		ssize_t res = iio_device_attr_write(m_iioDevices[SWIOT_DEVICE_NAME], "mode", "config");
		if (res >= 0) {
			qInfo(CAT_SWIOT) << "Successfully changed the swiot mode to config";
		} else {
			qCritical(CAT_SWIOT) << "Error, could not change swiot mode to config";
		}
	} else {
		qCritical(CAT_SWIOT) << "Error, could not find swiot device to change config mode";
	}

	Q_EMIT backBtnPressed();
}
