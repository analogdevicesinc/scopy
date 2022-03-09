#include "datalogger.hpp"


using namespace adiscope;

DataLogger::DataLogger(bool lastValue, bool average, bool all)
{
	dataLoggerAPI = new DataLoggerAPI();
	dataLoggerUI = new DataLoggerUI(lastValue,average,all);

	connect(dataLoggerUI, &DataLoggerUI::pathChanged, dataLoggerAPI, &DataLoggerAPI::setPath);
	connect(dataLoggerUI, &DataLoggerUI::timeIntervalChanged, dataLoggerAPI, &DataLoggerAPI::setTimerInterval);
	connect(dataLoggerUI, &DataLoggerUI::toggleDataLogger, this, &DataLogger::dataLoggerToggled);

	dataLoggerAPI->setTimerInterval(5000);
	isRunningOn = false;
}

void DataLogger::startLogger()
{
  dataLoggerAPI->startLogger(dataLoggerUI->isOverwrite());
}

void DataLogger::stopLogger()
{
	dataLoggerAPI->stopLogger();
}

void DataLogger::createChannel(QString name, Type type)
{
	ch_Filter filter =ch_Filter::LAST_VALUE ;
	QString uiFilter = dataLoggerUI->getFilter();
	if(uiFilter == "Average"){
		filter = ch_Filter::AVERAGE;
	}
	if(uiFilter == "All"){
		filter = ch_Filter::ALL;
	}
	dataLoggerAPI->createChannel(name,type,filter);
}

void DataLogger::distroyChannel(QString name)
{
	dataLoggerAPI->distroyChannel(name);
}

void DataLogger::resetChannel(QString name)
{
	dataLoggerAPI->resetChannel(name);
}

void DataLogger::receiveValue(QString name, QString value)
{
	dataLoggerAPI->receiveValue(name,value);
}

QWidget* DataLogger::getWidget()
{
	return dataLoggerUI->getDataLoggerUIWidget();
}

bool DataLogger::isDataLoggerOn()
{
	return dataLoggerUI->isDataLoggingOn();
}

void DataLogger::setWarningMessage(QString message)
{
	dataLoggerUI->setWarningMessage(message);
}

void DataLogger::setIsRunningOn(bool newIsRunningOn)
{
	isRunningOn = newIsRunningOn;
	if (isRunningOn) {
		atteptDataLogging();
	} else {
		dataLoggerUI->disableDataLogging(false);
	}
}

void DataLogger::dataLoggerToggled(bool toggled)
{
	dataLoggerUI->toggleDataLoggerSwitch(toggled);
	if (isRunningOn && !toggled) {
		dataLoggerUI->disableDataLogging(true);
		Q_EMIT isDataLogging(false);
	}
}

void DataLogger::atteptDataLogging(){
	if (dataLoggerUI->isDataLoggingOn() && !dataLoggerUI->getDataLoggerPath().isEmpty()) {
		startLogger();
		Q_EMIT isDataLogging(true);
	} else {
		stopLogger();
		Q_EMIT isDataLogging(false);
		dataLoggerUI->disableDataLogging(true);
		dataLoggerUI->toggleDataLoggerSwitch(false);
	}
}

void DataLogger::setPath(QString path){
	dataLoggerUI->setDataLoggerPath(path);
	dataLoggerAPI->setPath(path);
}

QString DataLogger::getPath(){
	return dataLoggerUI->getDataLoggerPath();
}

bool DataLogger::isOverwrite()
{
	return dataLoggerUI->isOverwrite();
}

void DataLogger::setOverwrite(bool en)
{
	dataLoggerUI->setOverwrite(en);
}

int DataLogger::getRecordingTimeInterval()
{
	return dataLoggerUI->getTimerInterval();
}

void DataLogger::setRecodingTimeInterval(int interval)
{
	dataLoggerUI->setTimerInterval(interval);
	dataLoggerAPI->setTimerInterval(interval * 1000);
}

void DataLogger::enableDataLogging(bool en)
{
	dataLoggerUI->enableDataLogging(en);
}

DataLogger::~DataLogger()
{
	delete dataLoggerAPI;
	delete dataLoggerUI;
}

