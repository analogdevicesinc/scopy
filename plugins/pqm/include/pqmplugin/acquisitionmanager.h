#ifndef ACQUISITIONMANAGER_H
#define ACQUISITIONMANAGER_H

#include "qtimer.h"
#include <iio.h>
#include <QObject>
#include <QMap>
#include <QFutureWatcher>

#include <iioutil/commandqueue.h>

#define MAX_ATTR_SIZE 1024
#define BUFFER_SIZE 1024
#define DEVICE_PQM "pqm"
#define SWAP_UINT32(x) (((x) >> 24) | (((x)&0x00FF0000) >> 8) | (((x)&0x0000FF00) << 8) | ((x) << 24))
namespace scopy::pqm {
class AcquisitionManager : public QObject
{
	Q_OBJECT
public:
	AcquisitionManager(iio_context *ctx, QObject *parent = nullptr);
	~AcquisitionManager();

	void readPqmAttributes();
	void readBufferedData();
	void futureReadData();
	void readData();
	void setData(QMap<QString, QMap<QString, QString>>);
public Q_SLOTS:
	void toolEnabled(bool en, QString toolName);
	void setConfigAttr(QMap<QString, QMap<QString, QString>>);
Q_SIGNALS:
	void pqmAttrsAvailable(QMap<QString, QMap<QString, QString>>);
	void bufferDataAvailable(QMap<QString, std::vector<double>>);

private Q_SLOTS:
	void onReadFinished();

private:
	void enableBufferChnls(iio_device *dev);
	QTimer *m_dataRefreshTimer;
	QMap<QString, bool> m_tools = {{"rms", false}, {"harmonics", false}, {"waveform", false}, {"settings", false}};
	iio_context *m_ctx;
	iio_buffer *m_buffer;
	QFutureWatcher<void> *m_readFw;
	QFutureWatcher<void> *m_setFw;
	QVector<QString> m_chnlsName;
	QMap<QString, QMap<QString, QString>> m_pqmAttr;
	QMap<QString, std::vector<double>> m_bufferData;
};
} // namespace scopy::pqm

#endif // ACQUISITIONMANAGER_H
