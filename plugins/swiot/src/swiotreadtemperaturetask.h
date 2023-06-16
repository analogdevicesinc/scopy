#ifndef SCOPY_SWIOTREADTEMPERATURETASK_H
#define SCOPY_SWIOTREADTEMPERATURETASK_H

#include <QThread>
#include <iio.h>

namespace scopy::swiot {
class SwiotReadTemperatureTask : public QThread {
	Q_OBJECT
public:
	explicit SwiotReadTemperatureTask(QString  uri, QObject* parent = nullptr);
	void run() override;

Q_SIGNALS:
	void newTemperature(double value);

private:
	QString m_uri;

};
} // scopy::swiot

#endif //SCOPY_SWIOTREADTEMPERATURETASK_H
