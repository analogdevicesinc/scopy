#ifndef SCOPY_EXTERNALPSREADERTHREAD_H
#define SCOPY_EXTERNALPSREADERTHREAD_H

#include <QThread>

#include <iio.h>

namespace scopy::swiot {
class ExternalPsReaderThread : public QThread {
	Q_OBJECT
public:
	explicit ExternalPsReaderThread(struct iio_device* device, QString attribute, QObject *parent = nullptr);
	void run() override;

Q_SIGNALS:
	void hasConnectedPowerSupply(bool ps);

private:
	struct iio_device* m_device;
	QString m_attribute;

};
} // scopy::swiot

#endif //SCOPY_EXTERNALPSREADERTHREAD_H
