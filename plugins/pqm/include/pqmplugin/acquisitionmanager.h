#ifndef ACQUISITIONMANAGER_H
#define ACQUISITIONMANAGER_H

#include "qtimer.h"
#include <iio.h>
#include <QObject>
#include <QMap>
#include <QFutureWatcher>

#include <iioutil/commandqueue.h>

#define MAX_ATTR_SIZE 1024
#define BUFFER_SIZE 256
#define DEVICE_PQM "pqm"
#define NEW_MEASUREMENT_ATTR "get_new_measurement"
namespace scopy::pqm {
class AcquisitionManager : public QObject
{
	Q_OBJECT
public:
	AcquisitionManager(iio_context *ctx, QObject *parent = nullptr);
	~AcquisitionManager();

public Q_SLOTS:
	void toolEnabled(bool en, QString toolName);
	void setConfigAttr(QMap<QString, QMap<QString, QString>>);
Q_SIGNALS:
	void pqmAttrsAvailable(QMap<QString, QMap<QString, QString>>);
	void bufferDataAvailable(QMap<QString, QVector<double>>);

private Q_SLOTS:
	void futureReadData();
	void futureBufferRead();
	void onReadFinished();
	void onReadBufferFinished();
	void onSetFinished();

private:
	int readGetNewMeasurement(iio_device *dev);
	bool isMeasurementAvailable(iio_device *dev);
	double convertFromHwToHost(int value, QString chnlId);
	void enableBufferChnls(iio_device *dev);
	void prepareForSet();
	void readData();
	bool readPqmAttributes();
	bool readBufferedData();
	void setData(QMap<QString, QMap<QString, QString>>);

	iio_context *m_ctx;
	iio_buffer *m_buffer;
	QTimer *m_dataRefreshTimer;
	QFutureWatcher<void> *m_readFw;
	QFutureWatcher<bool> *m_readBufferFw;
	QFutureWatcher<void> *m_setFw;

	QVector<QString> m_chnlsName;
	QMap<QString, QMap<QString, QString>> m_pqmAttr;
	QMap<QString, QVector<double>> m_bufferData;
	QMap<QString, bool> m_tools = {{"rms", false}, {"harmonics", false}, {"waveform", false}, {"settings", false}};

	bool m_attrHaveBeenRead = false;
};
} // namespace scopy::pqm

#endif // ACQUISITIONMANAGER_H
