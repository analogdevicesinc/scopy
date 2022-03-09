#include "dataloggercontroller.hpp"

using namespace adiscope;

DataLoggerController::DataLoggerController(bool lastValue, bool average, bool all)
{
	dataLoggerModel = new DataLoggerModel();
	dataLoggerView = new DataLoggerView(lastValue,average,all);

	connect(dataLoggerView, &DataLoggerView::pathChanged, dataLoggerModel, &DataLoggerModel::setPath);
	connect(dataLoggerView, &DataLoggerView::timeIntervalChanged, dataLoggerModel, &DataLoggerModel::setTimerInterval);
	connect(dataLoggerView, &DataLoggerView::toggleDataLogger, this, &DataLoggerController::dataLoggerToggled);

	dataLoggerModel->setTimerInterval(5000);
	isRunningOn = false;
}

void DataLoggerController::startLogger()
{
	dataLoggerModel->startLogger(dataLoggerView->isOverwrite());
}

void DataLoggerController::stopLogger()
{
	dataLoggerModel->stopLogger();
}

void DataLoggerController::createChannel(QString name, CHANNEL_DATA_TYPE type)
{
	CHANNEL_FILTER filter =CHANNEL_FILTER::LAST_VALUE ;
	QString uiFilter = dataLoggerView->getFilter();
	if (uiFilter == "Average") {
		filter = CHANNEL_FILTER::AVERAGE;
	}
	if (uiFilter == "All") {
		filter = CHANNEL_FILTER::ALL;
	}
	dataLoggerModel->createChannel(name,type,filter);
}

void DataLoggerController::destroyChannel(QString name)
{
	dataLoggerModel->destroyChannel(name);
}

void DataLoggerController::resetChannel(QString name)
{
	dataLoggerModel->resetChannel(name);
}

void DataLoggerController::receiveValue(QString name, QString value)
{
	dataLoggerModel->receiveValue(name,value);
}

QWidget* DataLoggerController::getWidget()
{
	return dataLoggerView->getDataLoggerViewWidget();
}

bool DataLoggerController::isDataLoggerOn()
{
	return dataLoggerView->isDataLoggingOn();
}

void DataLoggerController::setWarningMessage(QString message)
{
	dataLoggerView->setWarningMessage(message);
}

void DataLoggerController::setIsRunningOn(bool newIsRunningOn)
{
	isRunningOn = newIsRunningOn;
	if (isRunningOn) {
		attemptDataLogging();
	} else {
		dataLoggerView->disableDataLogging(false);
	}
}

void DataLoggerController::dataLoggerToggled(bool toggled)
{
	dataLoggerView->toggleDataLoggerSwitch(toggled);
	if (isRunningOn && !toggled) {
		dataLoggerView->disableDataLogging(true);
		Q_EMIT isDataLogging(false);
	}
}

void DataLoggerController::attemptDataLogging(){
	if (dataLoggerView->isDataLoggingOn() && !dataLoggerView->getDataLoggerPath().isEmpty()) {
		startLogger();
		Q_EMIT isDataLogging(true);
	} else {
		stopLogger();
		Q_EMIT isDataLogging(false);
		dataLoggerView->disableDataLogging(true);
		dataLoggerView->toggleDataLoggerSwitch(false);
	}
}

void DataLoggerController::setPath(QString path)
{
	dataLoggerView->setDataLoggerPath(path);
	dataLoggerModel->setPath(path);
}

QString DataLoggerController::getPath()
{
	return dataLoggerView->getDataLoggerPath();
}

bool DataLoggerController::isOverwrite()
{
	return dataLoggerView->isOverwrite();
}

void DataLoggerController::setOverwrite(bool en)
{
	dataLoggerView->setOverwrite(en);
}

int DataLoggerController::getRecordingTimeInterval()
{
	return dataLoggerView->getTimerInterval();
}

void DataLoggerController::setRecodingTimeInterval(int interval)
{
	dataLoggerView->setTimerInterval(interval);
	dataLoggerModel->setTimerInterval(interval * 1000);
}

DataLoggerController::~DataLoggerController()
{
	delete dataLoggerModel;
	delete dataLoggerView;
}

