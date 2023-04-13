#ifndef M2KREADTEMPERATURETASK_H
#define M2KREADTEMPERATURETASK_H

#include <QObject>
#include <QThread>
#include <iio.h>

namespace scopy::m2k {
class M2kReadTemperatureTask : public QThread
{
	Q_OBJECT
public:
	M2kReadTemperatureTask(QString uri);
	void run() override;

Q_SIGNALS:
	void newTemperature(double);
private:
	QString m_uri;

};
}
#endif // M2KREADTEMPERATURETASK_H
