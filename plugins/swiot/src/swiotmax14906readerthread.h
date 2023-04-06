#ifndef SCOPY_SWIOTMAX14906READERTHREAD_H
#define SCOPY_SWIOTMAX14906READERTHREAD_H

#include <QThread>
#include <iio.h>
#include <QMap>

namespace adiscope {

class MaxReaderThread : public QThread {
	Q_OBJECT
public:
	explicit MaxReaderThread();
	~MaxReaderThread();

	void addChannel(int index, struct iio_channel* channel);
	void toggleChannel(int index, bool toggled = true);
	bool isChannelToggled(int index);
	void singleRun();

Q_SIGNALS:
	void channelDataChanged(int channelId, double value);

private:
	void run() override;

	QMap<int, QPair<bool, struct iio_channel*>> m_channels;
};

} // adiscope

#endif //SCOPY_SWIOTMAX14906READERTHREAD_H
