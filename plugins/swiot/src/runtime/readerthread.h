#ifndef READERTHREAD_H
#define READERTHREAD_H

#include <QThread>
#include <QMap>
#include <QMutex>
#include <atomic>
#include <iio.h>
#include <iioutil/commandqueue.h>

namespace scopy::swiot {
class ReaderThread : public QThread {
	Q_OBJECT
public:
	explicit ReaderThread(bool isBuffered, CommandQueue *cmdQueue, QObject *parent = nullptr);
	~ReaderThread();

	void addDioChannel(int index, struct iio_channel *channel);

	void addBufferedDevice(iio_device *device);

	void runDio();
        void singleDio();

	void runBuffered(int requiredBuffersNumber = 0);

	void createIioBuffer();

	void destroyIioBuffer();

	void enableIioChnls();

	int getEnabledChnls();

	QVector<std::pair<double, double>> getOffsetScaleVector();

	double convertData(unsigned int data, int idx);

	void startCapture(int requiredBuffersNumber = 0);

	void requestStop();
public Q_SLOTS:

	void onChnlsChange(QMap<int, struct chnlInfo *> chnlsInfo);
	void onSamplingFreqWritten(int samplingFreq);

Q_SIGNALS:

	void bufferRefilled(QVector<QVector<double>> bufferData, int bufferCounter);

	void channelDataChanged(int channelId, double value);

private Q_SLOTS:
	void bufferRefillCommandFinished(scopy::Command *cmd);
	void bufferCreateCommandFinished(scopy::Command *cmd);
	void bufferDestroyCommandFinished(scopy::Command *cmd);

private:
	void createBufferRefillCommand();
	void createBufferDestroyCommand();

        void run() override;

	bool isBuffered;
	QMap<int, struct iio_channel *> m_dioChannels;

	int m_samplingFreq = 4800;
	int m_enabledChnlsNo;
	std::atomic<int> bufferCounter;
	std::atomic<int> m_requiredBuffersNumber;
	std::condition_variable m_cond, m_condBufferCreated;

	CommandQueue *m_cmdQueue;
	QVector<scopy::Command*> m_dioChannelsReadCommands;
	Command *m_refillBufferCommand, *m_createBufferCommand;
	Command *m_destroyBufferCommand;

	struct iio_device *m_iioDev;
	struct iio_buffer *m_iioBuff;
	QMap<int, struct chnlInfo *> m_chnlsInfo;
	QVector<QVector<double>> m_bufferData;
	QVector<std::pair<double, double>> m_offsetScaleValues;

	std::mutex m_mutex;
};
}

#endif // READERTHREAD_H
