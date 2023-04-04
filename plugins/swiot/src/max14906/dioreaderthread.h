#ifndef SCOPY_SWIOTMAX14906READERTHREAD_H
#define SCOPY_SWIOTMAX14906READERTHREAD_H

#include <QThread>
#include <iio.h>
#include <QMap>

namespace adiscope::swiot {

class DioReaderThread : public QThread {
	Q_OBJECT
public:
	explicit DioReaderThread();
	~DioReaderThread() override;

	void addChannel(int index, struct iio_channel* channel);
	void toggleChannel(int index, bool toggled = true);
	bool isChannelToggled(int index);
	void singleRun();
        void setOutputValue(int index, bool value);

Q_SIGNALS:
	void channelDataChanged(int channelId, double value);

private:
	void run() override;

	QMap<int, QPair<bool, struct iio_channel*>> m_channels;
        QMap<int, bool> outputValues;
};

} // adiscope

#endif //SCOPY_SWIOTMAX14906READERTHREAD_H
