#ifndef SCOPY_SWIOTDIGITALCHANNELCONTROLLER_H
#define SCOPY_SWIOTDIGITALCHANNELCONTROLLER_H

#include "swiotdigitalchannel.h"
#include <core/logging_categories.h>


#include <QObject>
#include <string>
#include <iio.h>

#define ATTR_BUFFER_LEN 200

namespace adiscope {
class DigitalChannelController : public QWidget {
	Q_OBJECT
public:

	explicit DigitalChannelController(struct iio_channel* channel, const QString& deviceName, const QString& deviceType, QWidget *parent);
	~DigitalChannelController() override;

	DigitalChannel *getDigitalChannel() const;

	void writeType();

private:
	DigitalChannel *m_digitalChannel;

	QString m_channelName;
	QString m_channelType; // output or input

	std::string m_iioAttrAvailableTypes; // iio attribute
	std::vector<std::string> m_availableTypes;

	std::string m_iioAttrType; // iio attribute
	std::string m_type;

	struct iio_channel* m_channel;
};

} // adiscope

#endif //SCOPY_SWIOTDIGITALCHANNELCONTROLLER_H
