#ifndef READERTHREAD_H
#define READERTHREAD_H

#include "src/runtime/ad74413r/chnlinfo.h"
#include <QThread>
#include <QMap>
#include <iio.h>
#include <QMutex>

namespace scopy::swiot {
class ReaderThread : public QThread {
	Q_OBJECT
public:
	explicit ReaderThread(bool isBuffered, QObject *parent = nullptr);
	~ReaderThread();

	void addDioChannel(int index, struct iio_channel *channel);

	void addBufferedDevice(iio_device *device);

	void runDio();
        void singleDio();

	void runBuffered();

	void createIioBuffer();

	void destroyIioBuffer();

	void enableIioChnls();

	int getEnabledChnls();
	QVector<ChnlInfo *> getEnabledBufferedChnls();
public Q_SLOTS:

	void onChnlsChange(QMap<int, ChnlInfo *> chnlsInfo);
	void onSamplingFreqWritten(int samplingFreq);

Q_SIGNALS:

	void bufferRefilled(QVector<QVector<double>> bufferData, int bufferCounter);

	void channelDataChanged(int channelId, double value);

private:
        void run() override;

	bool isBuffered;
	QMap<int, struct iio_channel *> m_dioChannels;

	int m_samplingFreq = 4800;
	int m_enabledChnlsNo;
	int bufferCounter = 0;

	struct iio_device *m_iioDev;
	struct iio_buffer *m_iioBuff;
	QMap<int, ChnlInfo*> m_chnlsInfo;
	QVector<ChnlInfo *> m_bufferedChnls;
	QVector<QVector<double>> m_bufferData;

	QMutex *lock;
};
}

#endif // READERTHREAD_H
