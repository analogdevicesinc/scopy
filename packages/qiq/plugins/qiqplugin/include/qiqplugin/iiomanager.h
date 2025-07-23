#ifndef IIOMANAGER_H
#define IIOMANAGER_H

#include "qiqutils.h"

#include <iio.h>
#include <QObject>
#include <QFutureWatcher>
#include <datawriter.h>
#include <inputconfig.h>

namespace scopy::qiqplugin {

union iio_buffer_hack
{
	struct iio_buffer *buffer;
	struct
	{
		const struct iio_device *dev;
		void *buffer;
		char padding[40];
	} * fields;
};

class IIOManager : public QObject
{
	Q_OBJECT
public:
	IIOManager(iio_context *ctx, QObject *parent = nullptr);
	~IIOManager();

	QMap<QString, QStringList> getAvailableChannels();

public Q_SLOTS:
	void startAcq(bool en);
	void onDataRequest();
	void onBufferParamsChanged(BufferParams params);

Q_SIGNALS:
	void inputFormatChanged(const InputConfig &config);
	void dataReady(QVector<QVector<double>> &inputData);

private:
	void init();
	void computeDevMap();
	void destroyBuffer();
	void readBuffer();
	int enChannels(QString deviceName, QStringList enChnls);
	void onReadBufferData();
	QStringList getChannelsFormat(iio_device *dev);
	double getSamplingFrequency(iio_device *dev);
	InputConfig createInputConfig(iio_device *dev, int channelCount, int64_t bufferSamplesSize);
	double calculateBufferSamples(iio_channel *ch, iio_buffer *buffer);
	void chnlRead(iio_channel *chnl, QByteArray &dst);
	QVector<double> toDouble(QByteArray dst);
	void readAllChannels(QString deviceName);

	iio_buffer *createMmapIioBuffer(struct iio_device *dev, size_t samples, void **originalBufferPtr = nullptr);

	iio_context *m_ctx;
	iio_buffer *m_buffer = nullptr;
	void *m_originalBufferPtr = nullptr;
	int m_bufferSamples = 0;
	DataWriter *m_dataWriter;
	QFutureWatcher<void> *m_readFw;
	QVector<QVector<double>> m_bufferData;
	QMap<QString, QMap<QString, iio_channel *>> m_devMap;
	const QString DEFAULT_FILE_PATH = scopy::config::executableFolderPath() + QDir::separator() + "data.in";
};

} // namespace scopy::qiqplugin

#endif // IIOMANAGER_H
