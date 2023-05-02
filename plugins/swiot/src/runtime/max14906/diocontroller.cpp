#include "diocontroller.h"
#include "src/swiot_logging_categories.h"
#include "max14906.h"

using namespace scopy::swiot;

DioController::DioController(struct iio_context* context_, QString deviceName) :
	m_deviceName(std::move(deviceName)),
	m_context(context_) {
	struct iio_device* dev = iio_context_find_device(this->m_context, MAX_NAME);
	this->m_device = dev;
}

DioController::~DioController() {

}

int DioController::getChannelCount() {
	unsigned int dev_count = iio_device_get_channels_count(this->m_device);
	return (int)(dev_count);
}

QString DioController::getChannelName(unsigned int index) {
	iio_channel *channel = iio_device_get_channel(this->m_device, index);
	if (channel == nullptr) {
		qCritical(CAT_SWIOT_MAX14906) << "Error when selecting channel with index" << index << ", returning empty string.";
		return "";
	}
	QString name = iio_channel_get_id(channel);

	return name;
}

QString DioController::getChannelType(unsigned int index) {
	iio_channel* channel = iio_device_get_channel(this->m_device, index);
	if (channel == nullptr) {
		qCritical(CAT_SWIOT_MAX14906) << "Error when selecting channel with index" << index << ", returning empty string.";
		return "";
	}
	bool output = iio_channel_is_output(channel);

	return output ? "output" : "input";
}

iio_device *DioController::getDevice() const {
	return DioController::m_device;
}
