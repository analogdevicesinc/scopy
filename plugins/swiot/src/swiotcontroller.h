#ifndef SWIOTCONTROLLER_H
#define SWIOTCONTROLLER_H

#include <QObject>
#include <iio.h>
#include "iioutil/iiopingtask.h"
#include "iioutil/cyclicaltask.h"
#include "src/swiotswitchctxtask.h"


namespace scopy::swiot {
class SwiotController : public QObject
{
	Q_OBJECT
public:
	SwiotController(QString uri, QObject *parent = nullptr);
	~SwiotController();

	void startPingTask();
	void stopPingTask();

//	void startPowerSupplyTask();
//	void stopPowerSupplyTask();

	void startSwitchContextTask();
	void stopSwitchContextTask();

	void connectSwiot(iio_context *ctx);
	void disconnectSwiot();

Q_SIGNALS:
	void pingSuccess();
	void pingFailed();
	void contextSwitched();

private:
	IIOPingTask *pingTask;
	SwiotSwitchCtxTask *switchCtxTask;
	iio_context *m_iioCtx;
	QString uri;

	CyclicalTask *pingTimer;
	CyclicalTask *switchCtxTimer;

};
}


#endif // SWIOTCONTROLLER_H
