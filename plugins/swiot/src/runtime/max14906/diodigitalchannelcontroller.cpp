#include "diodigitalchannelcontroller.h"
#include "src/swiot_logging_categories.h"
#include <sstream>

using namespace scopy::swiot;

DioDigitalChannelController::DioDigitalChannelController(struct iio_channel* channel, const QString &deviceName, const QString &deviceType, QWidget *parent) :
	QWidget(parent),
	m_digitalChannel(new DioDigitalChannel(deviceName, deviceType, this)),
	m_channelName(deviceName),
	m_channelType(deviceType),
	m_channel(channel)
{
	this->m_iioAttrAvailableTypes = (this->m_channelType == "INPUT") ? "IEC_type_available" : "do_mode_available";
	this->m_iioAttrType = (this->m_channelType == "INPUT") ? "IEC_type" : "do_mode";

	char buffer[ATTR_BUFFER_LEN];
	ssize_t readResult = iio_channel_attr_read(this->m_channel, this->m_iioAttrAvailableTypes.c_str(), buffer, ATTR_BUFFER_LEN);

	if (readResult < 0) {
		qCritical(CAT_SWIOT_MAX14906) << "Could not read the available types, error code:" << readResult;
	}

	// separate the available types
	std::stringstream ss(buffer);
	std::string aux;
	while (std::getline(ss, aux, ' ')) {
		this->m_availableTypes.push_back(aux);
	}

	memset(buffer, 0, ATTR_BUFFER_LEN);
	readResult = iio_channel_attr_read(this->m_channel, this->m_iioAttrType.c_str(), buffer, ATTR_BUFFER_LEN);

	if (readResult < 0) {
		qCritical(CAT_SWIOT_MAX14906) << "Could not read the type, error code:" << readResult;
	}

	this->m_type = buffer;

	this->m_digitalChannel->setConfigModes(&this->m_availableTypes);
	this->m_digitalChannel->setSelectedConfigMode(QString::fromStdString(this->m_type));

	if (deviceType == "OUTPUT") {
		bool valueRead;
		int res = iio_channel_attr_read_bool(channel, "raw", &valueRead);

		if (res < 0) {
			qCritical(CAT_SWIOT_MAX14906) << "Could not read initial channel raw value, error code:" << readResult;
		} else {
			m_digitalChannel->ui->customSwitch->setChecked(valueRead);
		}
	}

	QObject::connect(this->m_digitalChannel->ui->configModes, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 [&](int index){
		this->writeType();
	});
	QObject::connect(m_digitalChannel, &DioDigitalChannel::outputValueChanged, this,
			 [this] (bool value) {
		ssize_t res = iio_channel_attr_write_bool(m_channel, "raw", value);
		if (res < 0) {
			qCritical(CAT_SWIOT_MAX14906) << "Could not write value" << value << "to channel" << this->m_channelName << ", error code " << res;
		}
	});
}

DioDigitalChannelController::~DioDigitalChannelController() = default;

DioDigitalChannel *DioDigitalChannelController::getDigitalChannel() const {
	return m_digitalChannel;
}

void DioDigitalChannelController::writeType() {
	QString s = this->m_digitalChannel->ui->configModes->currentText();
	this->m_type = s.toStdString();
	qDebug(CAT_SWIOT_MAX14906) << "Writing <" << s << "> to channel <" << this->m_channelName << "> ";

	ssize_t res = iio_channel_attr_write(this->m_channel, this->m_iioAttrType.c_str(), s.toStdString().c_str());
	if (res < 0) {
		qCritical(CAT_SWIOT_MAX14906) << "Could not write attr to channel " << this->m_channelName << ", error code:" << res;
	}
}
