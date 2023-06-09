#ifndef SCOPY_EXTERNALPSREADERTHREAD_H
#define SCOPY_EXTERNALPSREADERTHREAD_H

#include <QThread>

#include <iio.h>

namespace scopy::swiot {
class ExternalPsReaderThread : public QThread {
	Q_OBJECT
public:
	explicit ExternalPsReaderThread(QString uri, QString attr);
	void run() override;

Q_SIGNALS:
	void hasConnectedPowerSupply(bool ps);

private:
	QString m_uri;
	QString m_attribute;

};
} // scopy::swiot

#endif //SCOPY_EXTERNALPSREADERTHREAD_H
