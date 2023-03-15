#include "m2kcontroller.h"
#include "m2kcommon.h"
#include <QFuture>
#include <QFutureWatcher>
#include <iioutil/contextprovider.h>

using namespace libm2k;
using namespace libm2k::analog;
using namespace libm2k::context;

using namespace adiscope;
using namespace adiscope::m2k;

M2kController::M2kController(QString uri, QObject *parent) : QObject(parent), uri(uri)
{	
	identifyTask = nullptr;
}

M2kController::~M2kController()
{

}

void M2kController::startPingTask()
{
	pingTask = new IIOPingTask(m_iioctx);
	pingTimer = new CyclicalTask(pingTask, this);
	connect(pingTask,SIGNAL(pingSuccess()),this,SIGNAL(pingSucces()));
	connect(pingTask,SIGNAL(pingFailed()),this,SIGNAL(pingFailed()));
	pingTimer->start();
}

void M2kController::stopPingTask()
{
	pingTask->requestInterruption();
	pingTask->deleteLater();
	pingTimer->deleteLater();
}

void M2kController::startTemperatureTask()
{
	tempTask = new M2kReadTemperatureTask(uri);
	tempTimer = new CyclicalTask(tempTask);
	connect(tempTask,SIGNAL(newTemperature(double)),this,SIGNAL(newTemperature(double)));
	tempTimer->start();
}

void M2kController::stopTemperatureTask()
{
	tempTimer->stop();
	tempTask->requestInterruption();
	disconnect(tempTask,SIGNAL(newTemperature(double)),this,SIGNAL(newTemperature(double)));
}

void M2kController::connectM2k(iio_context *ctx)
{
	m_iioctx = ctx;
	m_m2k = m2kOpen(ctx,"");
	identify();
}

void M2kController::disconnectM2k()
{

	try {
	contextClose(m_m2k,true);
	} catch(std::exception &ex) {
		qDebug(CAT_M2KPLUGIN)<<ex.what();

	}
	m_iioctx = nullptr;
	m_m2k = nullptr;
}


void M2kController::identify()
{
	if(!identifyTask) {
		identifyTask = new M2kIdentifyTask(uri);
		identifyTask->start();
		connect(identifyTask,&QThread::finished,this,[=](){
			delete identifyTask;
			identifyTask = nullptr;
		});
	}
}

void M2kController::initialCalibration()
{
	if(!m_m2k->isCalibrated())  {
		calibrate();
	} else {
		// already calibrated / or skipped
	}
}

void M2kController::calibrate()
{

	QFutureWatcher<bool> *fw = new QFutureWatcher<bool>(this);
	QFuture<bool> f = QtConcurrent::run(std::bind(&libm2k::context::M2k::calibrate,m_m2k));
	connect(fw,&QFutureWatcher<bool>::finished,this,[=](){
		if(fw->result()) {
			Q_EMIT calibrationSuccess();
		} else  {
			Q_EMIT calibrationFailed();
		}
		fw->deleteLater();
	});
	fw->setFuture(f);

	Q_EMIT calibrationStarted();

}




