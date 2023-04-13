#ifndef M2KCONTROLLER_H
#define M2KCONTROLLER_H

#include <QObject>
#include <QtConcurrent/QtConcurrent>
#include "m2kreadtemperaturetask.h"
#include "m2kidentifytask.h"
#include "iioutil/iiopingtask.h"
#include "iioutil/cyclicaltask.h"

#include "libm2k/m2k.hpp"
#include <libm2k/contextbuilder.hpp>


namespace scopy::m2k {
class M2kController : public QObject
{
	Q_OBJECT
public:
	M2kController(QString uri, QObject *parent = nullptr);
	~M2kController();

	void startPingTask();
	void stopPingTask();

	void startTemperatureTask();
	void stopTemperatureTask();

	void connectM2k(iio_context *ctx);
	void disconnectM2k();

public Q_SLOTS:
	void identify();
	void initialCalibration();
	void calibrate();

Q_SIGNALS:
	void newTemperature(double);
	void pingSuccess();
	void pingFailed();

	void calibrationStarted();
	void calibrationSuccess();
	void calibrationFailed();
	void calibrationFinished();


private:
	M2kReadTemperatureTask *tempTask;
	M2kIdentifyTask *identifyTask;
	IIOPingTask *pingTask;
	iio_context *m_iioctx;
	QString uri;
	libm2k::context::M2k *m_m2k;

	CyclicalTask *tempTimer;
	CyclicalTask *pingTimer;
};


}
#endif // M2KCONTROLLER_H
