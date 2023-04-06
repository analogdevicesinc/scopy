#ifndef SWIOTADREADERTHREAD_HPP
#define SWIOTADREADERTHREAD_HPP
#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

#include <QObject>
#include <qthread.h>
#include <iio.h>
#include <errno.h>

extern "C" {
	struct iio_device;
	struct iio_buffer;
}

class SwiotAdReaderThread: public QThread
{
	Q_OBJECT
public:
	SwiotAdReaderThread();
	double convertData(unsigned int data, int idx);
public Q_SLOTS:
	void onBufferCreated(struct iio_buffer* iioBuff, int enableChnlsNo, std::vector<std::pair<double, double>> offsetScaleValues);
	void onBufferDestroyed();
Q_SIGNALS:
	void bufferRefilled(QVector<QVector<double>> bufferData);
private:
	void run() override;

	struct iio_buffer* m_iioBuff;
	QVector<QVector<double>> m_bufferData;
	std::vector<std::pair<double, double>> m_offsetScaleValues;
	int m_enabledChnlsNo;
};

#endif // SWIOTADREADERTHREAD_HPP
