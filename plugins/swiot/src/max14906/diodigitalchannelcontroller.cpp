#include "diodigitalchannelcontroller.hpp"
#include <sstream>

using namespace adiscope::swiot;

DioDigitalChannelController::DioDigitalChannelController(struct iio_channel* channel, const QString &deviceName, const QString &deviceType, QWidget *parent) :
	QWidget(parent),
	m_digitalChannel(new DioDigitalChannel(deviceName, deviceType, this)),
	m_channelName(deviceName),
	m_channelType(deviceType),
	m_channel(channel)
{
	this->m_iioAttrAvailableTypes = (this->m_channelType == "input") ? "IEC_type_available" : "do_mode_available";
	this->m_iioAttrType = (this->m_channelType == "input") ? "IEC_type" : "do_mode";

	char buffer[ATTR_BUFFER_LEN];
	iio_channel_attr_read(this->m_channel, this->m_iioAttrAvailableTypes.c_str(), buffer, ATTR_BUFFER_LEN);

	std::stringstream ss(buffer);
	std::string aux;
	while (std::getline(ss, aux, ' ')) {
		this->m_availableTypes.push_back(aux);
	}

	memset(buffer, 0, ATTR_BUFFER_LEN);
	iio_channel_attr_read(this->m_channel, this->m_iioAttrType.c_str(), buffer, ATTR_BUFFER_LEN);
	this->m_type = buffer;

	this->m_digitalChannel->setConfigModes(this->m_availableTypes);
	this->m_digitalChannel->setSelectedConfigMode(QString::fromStdString(this->m_type));

	QObject::connect(this->m_digitalChannel->getUi()->configModes, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 [&](int index){
		this->writeType();
	});
	QObject::connect(m_digitalChannel, &DioDigitalChannel::outputValueChanged, this,
			 [this] (bool value) {
		ssize_t res = iio_channel_attr_write_bool(m_channel, "raw", value);
		if (res < 0) {
			qCritical(CAT_MAX14906) << "Could not write value" << value << "to channel" << this->m_channelName << ", error code " << res;
		}
	});
}

DioDigitalChannelController::~DioDigitalChannelController() {

}

DioDigitalChannel *DioDigitalChannelController::getDigitalChannel() const {
	return m_digitalChannel;
}

void DioDigitalChannelController::writeType() {
	QString s = this->m_digitalChannel->getUi()->configModes->currentText();
	this->m_type = s.toStdString();
	qDebug(CAT_MAX14906) << "Writing <" << s << "> to channel <" << this->m_channelName << "> ";

	ssize_t res = iio_channel_attr_write(this->m_channel, this->m_iioAttrType.c_str(), s.toStdString().c_str());
	if (res < 0) {
		qCritical(CAT_MAX14906) << "Could not write attr to channel " << this->m_channelName << ", error code " << res;
	}
}
