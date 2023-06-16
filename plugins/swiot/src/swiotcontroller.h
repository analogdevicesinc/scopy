#ifndef SWIOTCONTROLLER_H
#define SWIOTCONTROLLER_H

#include <QObject>
#include <iio.h>
#include "iioutil/iiopingtask.h"
#include "iioutil/cyclicaltask.h"
#include "src/externalpsreaderthread.h"
#include "src/swiotswitchctxtask.h"
#include "src/swiotreadtemperaturetask.h"


namespace scopy::swiot {
class SwiotController : public QObject
{
	Q_OBJECT
public:
	SwiotController(QString uri, QObject *parent = nullptr);
	~SwiotController();

	void startPingTask();
	void stopPingTask();

	void startPowerSupplyTask(QString attribute);
	void stopPowerSupplyTask();

	void startSwitchContextTask(bool isRuntime);
	void stopSwitchContextTask();

	void startTemperatureTask();
	void stopTemperatureTask();

	void connectSwiot(iio_context *ctx);
	void disconnectSwiot();

Q_SIGNALS:
	void pingSuccess();
	void pingFailed();
	void contextSwitched();
	void hasConnectedPowerSupply(bool ps);
	void readTemperature(double temperature);

private:
	IIOPingTask *pingTask;
	SwiotSwitchCtxTask *switchCtxTask;
	ExternalPsReaderThread *extPsTask;
	SwiotReadTemperatureTask *temperatureTask;
	iio_context *m_iioCtx;
	QString uri;

	CyclicalTask *pingTimer;
	CyclicalTask *switchCtxTimer;
	CyclicalTask *powerSupplyTimer;
	CyclicalTask *temperatureTimer;
};
}


#endif // SWIOTCONTROLLER_H
