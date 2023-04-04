#ifndef SCOPY_DIOMAX14906CONTROLLER_HPP
#define SCOPY_DIOMAX14906CONTROLLER_HPP

#include <QString>
#include <utility>
#include <iio.h>
#include <QThread>
#include <QTimer>

namespace adiscope::swiot {
class DioController : public QObject {
	Q_OBJECT
public:
	explicit DioController(struct iio_context* context_, QString deviceName = "max14906");
	~DioController() override;

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

#endif //SCOPY_DIOMAX14906CONTROLLER_HPP
