#ifndef SWIOTPINGTASK_H
#define SWIOTPINGTASK_H

#include <QThread>
#include <iio.h>
#include "scopy-swiot_export.h"
#include <iioutil/command.h>

namespace scopy::swiot {
class SwiotPingTask : public QThread {
	Q_OBJECT
public:
	SwiotPingTask(iio_context *c, QObject *parent = nullptr);
	~SwiotPingTask();
	virtual void run() override;
Q_SIGNALS:
	void pingSuccess();
	void pingFailed();
protected:
	iio_context *c;
	bool enabled;

private Q_SLOTS:
	void getTriggerCommandFinished(scopy::Command *cmd);
};
}
#endif // SWIOTPINGTASK_H
