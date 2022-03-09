#include "datalogger_api.h"
#include "gui/customSwitch.hpp"
#include "gui/dataloggercontroller.hpp"

using namespace adiscope;

bool DataLogger_API::get_show_all()
{
	return dataLogger->showAllSWitch->isChecked();
}

void DataLogger_API::set_show_all(bool en)
{
	dataLogger->showAllSWitch->setChecked(en);
}

int DataLogger_API::get_precision()
{
	return dataLogger->getPrecision();
}

void DataLogger_API::set_precision(int precision)
{
	dataLogger->setPrecision(precision);
	Q_EMIT dataLogger->precisionChanged(precision);
}

int DataLogger_API::get_recording_interval()
{
	return dataLogger->getValueReadingTimeInterval();
}

void DataLogger_API::set_recording_interval(int interval)
{
	Q_EMIT dataLogger->recordingIntervalChanged(interval);
}

bool DataLogger_API::get_data_logging_on(){
	return dataLogger->dataLogger->isDataLoggerOn();
}
void DataLogger_API::set_data_logging_on(bool en){
	dataLogger->dataLogger->dataLoggerToggled(en);
}

QString DataLogger_API::get_data_logging_path()
{
	return dataLogger->dataLogger->getPath();
}

void DataLogger_API::set_data_logging_path(QString path)
{
	if (dataLogger->dataLogger->isDataLoggerOn()) {
		dataLogger->dataLogger->setPath(path);
	} else {
		dataLogger->dataLogger->setPath("");
	}
}

bool DataLogger_API::get_data_logger_overwrite_append()
{
	return dataLogger->dataLogger->isOverwrite();
}

void DataLogger_API::set_data_logger_overwrite_append(bool en)
{
	if (dataLogger->dataLogger->isDataLoggerOn()) {
		dataLogger->dataLogger->setOverwrite(en);
	}
}

int DataLogger_API::get_data_logger_recording_interval()
{
	return dataLogger->dataLogger->getRecordingTimeInterval();
}

void DataLogger_API::set_data_logger_recording_interval(int interval)
{
	if (dataLogger->dataLogger->isDataLoggerOn()) {
		dataLogger->dataLogger->setRecodingTimeInterval(interval);
	}
}
