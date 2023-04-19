#ifndef DATALOGGER_API_H
#define DATALOGGER_API_H

#include <QObject>
#include "apiObject.hpp"
#include "datalogger.hpp"

namespace adiscope {

class DataLoggerController;
class CustomSwitch;
class DataLogger;

class DataLogger_API:public ApiObject
{
	friend class ToolLauncher_API;

	Q_OBJECT

	Q_PROPERTY(bool show_all
			   READ get_show_all WRITE set_show_all);
	Q_PROPERTY(int precision
			   READ get_precision WRITE set_precision);
	Q_PROPERTY(int recording_interval
			   READ get_recording_interval WRITE set_recording_interval);
	Q_PROPERTY(bool data_logging_on
			   READ get_data_logging_on WRITE set_data_logging_on);
	Q_PROPERTY(QString data_logging_path
			   READ get_data_logging_path WRITE set_data_logging_path);
	Q_PROPERTY(bool data_logger_overwrite_append
			   READ get_data_logger_overwrite_append WRITE set_data_logger_overwrite_append);
	Q_PROPERTY(int data_logger_recording_interval
			   READ get_data_logger_recording_interval WRITE set_data_logger_recording_interval);
public:

	bool get_show_all();
	void set_show_all(bool en);

	int get_precision();
	void set_precision(int precision);

	int get_recording_interval();
	void set_recording_interval(int interval);

	bool get_data_logging_on();
	void set_data_logging_on(bool en);

	QString get_data_logging_path();
	void set_data_logging_path(QString path);

	bool get_data_logger_overwrite_append();
	void set_data_logger_overwrite_append(bool en);

	int get_data_logger_recording_interval();
	void set_data_logger_recording_interval(int interval);

	explicit DataLogger_API(DataLogger *dataLogger) : ApiObject(), dataLogger(dataLogger) {};

private:
	DataLogger *dataLogger;
};
}
#endif // DATALOGGER_API_H
