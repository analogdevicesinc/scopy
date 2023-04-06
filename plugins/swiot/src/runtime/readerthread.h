#ifndef READERTHREAD_H
#define READERTHREAD_H

#include <QThread>
#include <QMap>
#include <iio.h>
#include <QMutex>

namespace adiscope::swiot {
class ReaderThread : public QThread {
	Q_OBJECT
public:
	explicit ReaderThread(bool isBuffered, QObject *parent = nullptr);

	void addDioChannel(int index, struct iio_channel *channel);

	void addBufferedDevice(iio_device *device);

	void runDio();
        void singleDio();

	void runBuffered();

	void createIioBuffer();

	void destroyIioBuffer();

	void enableIioChnls();

	int getEnabledChnls();

	QVector<std::pair<double, double>> getOffsetScaleVector();

	double convertData(unsigned int data, int idx);

public Q_SLOTS:

	void onChnlsChange(QMap<int, struct chnlInfo *> chnlsInfo);

Q_SIGNALS:

	void bufferRefilled(QVector<QVector<double>> bufferData, int bufferCounter);

	void channelDataChanged(int channelId, double value);

private:
        void run() override;

	bool isBuffered;
	QMap<int, struct iio_channel *> m_dioChannels;

	int m_sampleRate = 4800;
	double m_timespan = 1;
	int m_enabledChnlsNo;
	int bufferCounter = 0;

	struct iio_device *m_iioDev;
	struct iio_buffer *m_iioBuff;
	QMap<int, struct chnlInfo *> m_chnlsInfo;
	QVector<QVector<double>> m_bufferData;
	QVector<std::pair<double, double>> m_offsetScaleValues;

	QMutex *lock;
};
}

#endif // READERTHREAD_H
