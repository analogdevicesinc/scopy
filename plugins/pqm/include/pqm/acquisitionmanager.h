#ifndef ACQUISITIONMANAGER_H
#define ACQUISITIONMANAGER_H

#include "qtimer.h"
#include <iio.h>
#include <QObject>
#include <QMap>
#include <QFutureWatcher>

#include <iioutil/commandqueue.h>
#include <iioutil/pingtask.h>

#define MAX_ATTR_SIZE 1024
#define BUFFER_SIZE 512
#define DEVICE_PQM "pqm"
namespace scopy::pqm {
class AcquisitionManager : public QObject
{
	Q_OBJECT
public:
	AcquisitionManager(iio_context *ctx, PingTask *pingTask, QObject *parent = nullptr);
	~AcquisitionManager();

	bool hasFwVers() const;

public Q_SLOTS:
	void toolEnabled(bool en, QString toolName);
	void setConfigAttr(QMap<QString, QMap<QString, QString>>);
	void startPing();
	void stopPing();
Q_SIGNALS:
	void pqmAttrsAvailable(QMap<QString, QMap<QString, QString>>);
	void bufferDataAvailable(QMap<QString, QVector<double>>);

private Q_SLOTS:
	void futureReadData();
	void onReadFinished();
	void pingTimerTimeout();

private:
	double convertFromHwToHost(int value, QString chnlId);
	void enableBufferChnls(iio_device *dev);
	void readData();
	bool readPqmAttributes();
	bool readBufferedData();
	void setData(QMap<QString, QMap<QString, QString>>);
	void setProcessData(bool en);

	iio_context *m_ctx;
	iio_buffer *m_buffer;

	QTimer *m_pingTimer = nullptr;
	PingTask *m_pingTask = nullptr;
	QFutureWatcher<void> *m_readFw;
	QFutureWatcher<void> *m_setFw;

	QMutex mutex;
	QVector<QString> m_chnlsName;
	QMap<QString, QMap<QString, QString>> m_pqmAttr;
	QMap<QString, QVector<double>> m_bufferData;
	QMap<QString, bool> m_tools = {{"rms", false}, {"harmonics", false}, {"waveform", false}, {"settings", false}};

	bool m_attrHaveBeenRead = false;
	bool m_buffHaveBeenRead = false;
	bool m_hasFwVers = false;
	const int THREAD_FINISH_TIMEOUT = 10000;
};
} // namespace scopy::pqm

#endif // ACQUISITIONMANAGER_H
