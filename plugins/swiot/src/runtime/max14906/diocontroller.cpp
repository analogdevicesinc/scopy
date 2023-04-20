#include "diocontroller.hpp"

using namespace scopy::swiot;

DioController::DioController(struct iio_context* context_, QString deviceName) :
	m_deviceName(std::move(deviceName)),
	m_context(context_) {
	struct iio_device* dev = iio_context_get_device(this->m_context, 1);
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
	QString name = iio_channel_get_id(channel);

	return name;
}

QString DioController::getChannelType(unsigned int index) {
	iio_channel* channel = iio_device_get_channel(this->m_device, index);
	bool output = iio_channel_is_output(channel);

	return output ? "output" : "input";
}

iio_device *DioController::getDevice() const {
	return DioController::m_device;
}
