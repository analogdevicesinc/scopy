#ifndef BUFFERLOGIC_H
#define BUFFERLOGIC_H

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

#define VOLTAGE_UM "V"
#define CURRENT_UM "mA"
#define RESISTANCE_UM "Ω"

#define VOLTAGE_LIMIT 5
#define CURRENT_LIMIT 25
#define RESISTANCE_UPPER_LIMIT 8191
#define RESISTANCE_LOWER_LIMIT 0

struct chnlInfo {
	QString chnlId;
	bool isOutput;
	bool isEnabled;
	bool isScanElement;
	QString unitOfMeasure;
	std::pair<int, int> rangeValues;
	std::pair<double, double> offsetScalePair;
	struct iio_channel *iioChnl;
};


class BufferLogic : public QObject {
	Q_OBJECT
public:
	explicit BufferLogic(struct iio_device *iioDev = nullptr);

	~BufferLogic();

	struct iio_channel *getIioChnl(int chnlIdx, bool outputPriority);

	bool verifyEnableChanges(std::vector<bool> enabledChnls);

	int getPlotChnlsNo();
	QVector<QString> getPlotChnlsUnitOfMeasure();
	QVector<std::pair<int, int>> getPlotChnlsRangeValues();
	QMap<int, QString> getPlotChnlsId();

	QStringList readChnlsSamplingFreqAttr(QString attrName);

public Q_SLOTS:
	void onSamplingFreqChanged(int idx);

Q_SIGNALS:
	void chnlsChanged(QMap<int, struct chnlInfo*> chnlsInfo);
	void samplingFreqWritten(int samplingFreq);
private:
	void createChannels();
	void initializeChnlsScaleInfo();

private:
	int m_plotChnlsNo;
	QStringList m_samplingFreqAvailable;

	QMap<int, struct chnlInfo *> m_chnlsInfo;
	QMap<QChar, QString> m_unitsOfMeasure;
	QMap<QChar, std::pair<double, double>> m_valuesRange;
	struct iio_device *m_iioDev;
};
}


#endif // BUFFERLOGIC_H
