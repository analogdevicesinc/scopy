#ifndef SWIOTADREADERTHREAD_HPP
#define SWIOTADREADERTHREAD_HPP
#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

#include <QObject>
#include <qthread.h>
#include <iio.h>
#include <errno.h>
#include <QVector>
#include "swiotadlogic.hpp"

extern "C" {
	struct iio_device;
	struct iio_buffer;
}

class SwiotAdReaderThread: public QThread
{
	Q_OBJECT
public:
	SwiotAdReaderThread(struct iio_device *iioDev);
	double convertData(unsigned int data, int idx);
public Q_SLOTS:
	void onChnlsChange(QMap<int, struct chnlInfo*> chnlsInfo);
Q_SIGNALS:
	void bufferRefilled(QVector<QVector<double>> bufferData);
private:
	void run() override;

	void createIioBuffer();
	void destroyIioBuffer();

	void enableIioChnls();
	int getEnabledChnls();

	QVector<std::pair<double, double>> getOffsetScaleVector();
private:
	int m_sampleRate = 4800;
	double m_timespan = 1;
	int m_enabledChnlsNo;
	bool m_activeChanges = false;

	struct iio_device *m_iioDev;
	struct iio_buffer *m_iioBuff;
	QMap<int, struct chnlInfo*> m_chnlsInfo;
	QVector<QVector<double>> m_bufferData;
	QVector<std::pair<double, double>> m_offsetScaleValues;

};

#endif // SWIOTADREADERTHREAD_HPP
