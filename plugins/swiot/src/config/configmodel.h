#ifndef CONFIGMODEL_H
#define CONFIGMODEL_H

#include "qobject.h"
#include <QMap>
#include <string>

extern "C"{
struct iio_channel;
struct iio_device;
}

namespace scopy::swiot {
class ConfigModel : public QObject {
	Q_OBJECT
public:
	explicit ConfigModel(struct iio_device* device, int channelId);
	~ConfigModel();

	QString readEnabled();
	void writeEnabled(const QString& enabled);

	QString readDevice();
	void writeDevice(const QString& device);

	QString readFunction();
	void writeFunction(const QString& function);

	QStringList readDeviceAvailable();
	QStringList readFunctionAvailable();

private:
	struct iio_device* m_device;
	int m_channelId;

	std::string m_enableAttribute;
	std::string m_functionAttribute;
	std::string m_functionAvailableAttribute;
	std::string m_deviceAttribute;
	std::string m_deviceAvailableAttribute;

	QString m_enabled;
	QStringList m_availableDevices;
	QStringList m_availableFunctions;
};
}
#endif // CONFIGMODEL_H
