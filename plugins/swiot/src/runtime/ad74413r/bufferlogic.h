#ifndef BUFFERLOGIC_H
#define BUFFERLOGIC_H

#include "chnlinfo.h"

#include <QObject>
#include <QMap>
#include <iio.h>
#include <cerrno>

extern "C" {
struct iio_context;
struct iio_device;
struct iio_channel;
}
Q_DECLARE_OPAQUE_POINTER(struct iio_buffer*)

namespace scopy::swiot {
#define MAX_BUFFER_SIZE 160
#define MIN_BUFFER_SIZE 5
#define SAMPLING_FREQ_ATTR_NAME "sampling_frequency"
#define MAX_INPUT_CHNLS_NO 8

class BufferLogic : public QObject {
	Q_OBJECT
public:
	explicit BufferLogic(QMap<QString, iio_device*> devicesMap = {});

	~BufferLogic();

	QMap<QString, iio_channel*> getIioChnl(int chnlIdx);

	bool verifyEnableChanges(std::vector<bool> enabledChnls);

	int getPlotChnlsNo();
	QVector<QString> getPlotChnlsUnitOfMeasure();
	QVector<std::pair<int, int>> getPlotChnlsRangeValues();
	QMap<int, QString> getPlotChnlsId();

	QStringList readChnlsSamplingFreqAttr(QString attrName);
	QVector<QString> getAd74413rChnlsFunctions();

public Q_SLOTS:
	void onSamplingFreqChanged(int idx);

Q_SIGNALS:
	void chnlsChanged(QMap<int,  ChnlInfo *> chnlsInfo);
	void samplingFreqWritten(int samplingFreq);
private:
	void createChannels();

private:
	int m_plotChnlsNo;
	QMap<QString, iio_device*> m_iioDevicesMap;
	QStringList m_samplingFreqAvailable;

	QMap<int, ChnlInfo *> m_chnlsInfo;
};
}


#endif // BUFFERLOGIC_H
