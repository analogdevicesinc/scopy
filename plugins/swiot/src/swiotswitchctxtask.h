#ifndef SWIOTSWITCHCTXTASK_H
#define SWIOTSWITCHCTXTASK_H

#include <QThread>
#include <QObject>
#include <iio.h>

namespace scopy::swiot {
class SwiotSwitchCtxTask : public QThread
{
	Q_OBJECT
public:
	SwiotSwitchCtxTask(QString uri, bool wasRuntime);
	void run() override;

Q_SIGNALS:
	void contextSwitched();
private:
	QString m_uri;
	bool m_wasRuntime;
};
}


#endif // SWIOTSWITCHCTXTASK_H
