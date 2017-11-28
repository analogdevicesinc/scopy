#ifndef REGMAPPARSER_H
#define REGMAPPARSER_H

#include <QObject>
#include <iio.h>
#include <QFile>
#include <QDebug>
#include <QDomDocument>

#define PCORE_VERSION_MAJOR(version) (version >> 16)

class RegmapParser : public QObject
{
	Q_OBJECT

public:
	explicit RegmapParser(QObject *parent = nullptr,
	                      struct iio_context *context = nullptr);
	void deviceXmlFileSelection(const QString *device, QString *filename,
	                            const QString source);
	int deviceXmlFileLoad(QString *filename);
	void regMapChooserInit(QString *device);
	QDomNode *getRegisterNode(const QString address);
	void setIioContext(struct iio_context *ctx);
	uint32_t readRegister(const QString *device, const uint32_t u8Address);
	void writeRegister(const QString *device, const uint32_t u8Address,
	                   const uint32_t value);
	uint32_t getLastAddress(void) const;

private:
	void findDeviceXmlFile(const QString *xmlsFolderPath, const QString *device,
	                       QString *filename);
	int pcoreGetVersion(const QString *device, int *pcoreMajor);
	bool xmlFileExists(char *filename);
	bool isInputDevice(const struct iio_device *dev);
	bool isOutputDevice(const struct iio_device *dev);
	bool deviceTypeGet(const struct iio_device *dev, int type);

private:
	struct iio_context *ctx;
	QString xmlsFolderPath;
	QFile file;
	QDomDocument doc;
	QDomNode node;
	QDomNode lastNode;
};

#endif // REGMAPPARSER_H
