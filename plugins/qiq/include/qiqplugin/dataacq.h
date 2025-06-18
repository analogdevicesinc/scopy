#ifndef DATAACQ_H
#define DATAACQ_H

#include <QDir>
#include <QObject>
#include "acqsetup.h"
#include <common/scopyconfig.h>
namespace scopy::qiqplugin {

class DataAcq : public QObject
{
	Q_OBJECT
public:
	DataAcq(iio_context *ctx, QObject *parent = nullptr);
	~DataAcq();

	void readDeviceData();
	void initDevicesAndChnls();
Q_SIGNALS:
	void contextInfo(const QMap<QString, QStringList> map);
	void dataAvailable(QVector<QVector<double>>, const int &dataSize, const QString &path);

public Q_SLOTS:
	void onConfigPressed(AcqSetup::AcqConfig config);

private:
	void destroyBuffer();
	void chnlRead(iio_channel *ch, short *dst);
	void createBuffer(iio_device *dev);
	void mapFile(const QString &filePath);
	void writeToMappedFile();

	int m_samples;
	int m_dataSize;
	QFile m_file;
	uchar *m_mappedData;
	iio_context *m_ctx;
	iio_buffer *m_buffer;
	QList<iio_channel *> m_chnls;
	QVector<QVector<double>> m_data;

	const QString FILE_PATH = scopy::config::settingsFolderPath() + QDir::separator() + "device_data.bin";
};

} // namespace scopy::qiqplugin

#endif // DATAACQ_H
