#ifndef M2KCONTROLLER_H
#define M2KCONTROLLER_H

#include "iioutil/cyclicaltask.h"
#include "iioutil/iiopingtask.h"
#include "libm2k/m2k.hpp"
#include "m2kidentifytask.h"
#include "m2kreadtemperaturetask.h"

#include <QObject>
#include <QtConcurrent/QtConcurrent>

#include <libm2k/contextbuilder.hpp>

namespace scopy::m2k {
class M2kController : public QObject
{
	Q_OBJECT
public:
	M2kController(QString uri, QObject *parent = nullptr);
	~M2kController();

	void startTemperatureTask();
	void stopTemperatureTask();

	void connectM2k(libm2k::context::M2k *m2k);
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
	QString uri;
	libm2k::context::M2k *m_m2k;

	CyclicalTask *tempTimer;
};

} // namespace scopy::m2k
#endif // M2KCONTROLLER_H
