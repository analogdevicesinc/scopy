#ifndef SCOPY_SWIOTMAX14906CONTROLLER_HPP
#define SCOPY_SWIOTMAX14906CONTROLLER_HPP

#include <QString>
#include <utility>
#include <iio.h>
#include <QThread>
#include <QTimer>

namespace adiscope {
class Max14906ToolController : public QObject {
	Q_OBJECT
public:
	explicit Max14906ToolController(struct iio_context* context_, QString deviceName = "max14906");
	~Max14906ToolController() override;

	int getChannelCount();

	iio_device *getDevice() const;

	QString getChannelName(unsigned int index);
	QString getChannelType(unsigned int index);
private:
	QString m_deviceName;
	struct iio_context* m_context;
	struct iio_device* m_device;
};
}

#endif //SCOPY_SWIOTMAX14906CONTROLLER_HPP
