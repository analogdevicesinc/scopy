#ifndef SCOPY_DIODIGITALCHANNELCONTROLLER_H
#define SCOPY_DIODIGITALCHANNELCONTROLLER_H

#include "diodigitalchannel.h"
#include <iioutil/commandqueue.h>

#include <string>
#include <iio.h>

#define ATTR_BUFFER_LEN 200

namespace scopy::swiot {
class DioDigitalChannelController : public QWidget {
	Q_OBJECT
public:

	explicit DioDigitalChannelController(struct iio_channel* channel, const QString& deviceName,
					     const QString& deviceType, CommandQueue *cmdQueue, QWidget *parent);
	~DioDigitalChannelController() override;

	DioDigitalChannel *getDigitalChannel() const;
private Q_SLOTS:
	void createWriteCurrentLimitCommand(int index);
	void createWriteRawCommand(bool value);
	void createWriteTypeCommand(int index);
private:
	DioDigitalChannel *m_digitalChannel;

	QString m_channelName;
	QString m_channelType; // output or input

	std::string m_iioAttrAvailableTypes; // iio attribute
	std::vector<std::string> m_availableTypes;

	std::string m_iioAttrType; // iio attribute
	std::string m_type;

	CommandQueue *m_cmdQueue;
	Command *m_writeCurrentLimitCmd, *m_writeRawCmd;
	Command *m_writeTypeCmd, *m_readAvailableTypeCmd;
	Command *m_readTypeCmd, *m_readRawCmd;
	Command *m_readCurrentLimitAvailableCmd;

	struct iio_channel* m_channel;
};

} // scopy

#endif //SCOPY_DIODIGITALCHANNELCONTROLLER_H
