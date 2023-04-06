#include "swiotmax14906controller.hpp"

using namespace adiscope;

adiscope::Max14906ToolController::Max14906ToolController(struct iio_context* context_, QString deviceName) :
	m_deviceName(std::move(deviceName)),
	m_context(context_) {
	struct iio_device* dev = iio_context_get_device(this->m_context, 1);
	this->m_device = dev;
}

adiscope::Max14906ToolController::~Max14906ToolController() {

}

int adiscope::Max14906ToolController::getChannelCount() {
	unsigned int dev_count = iio_device_get_channels_count(this->m_device);
	return (int)(dev_count);
}

QString adiscope::Max14906ToolController::getChannelName(unsigned int index) {
	iio_channel *channel = iio_device_get_channel(this->m_device, index);
	QString name = iio_channel_get_id(channel);

	return name;
}

QString adiscope::Max14906ToolController::getChannelType(unsigned int index) {
	iio_channel* channel = iio_device_get_channel(this->m_device, index);
	bool output = iio_channel_is_output(channel);

	return output ? "output" : "input";
}

iio_device *adiscope::Max14906ToolController::getDevice() const {
	return Max14906ToolController::m_device;
}
