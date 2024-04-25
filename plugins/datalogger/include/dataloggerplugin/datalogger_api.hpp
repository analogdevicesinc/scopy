#ifndef DATALOGGER_API_H
#define DATALOGGER_API_H

#include "scopy-dataloggerplugin_export.h"

#include <dataloggerplugin.h>

namespace scopy::datamonitor {

class SCOPY_DATALOGGERPLUGIN_EXPORT DataLogger_API : public ApiObject
{
	Q_OBJECT
public:
	explicit DataLogger_API(DataLoggerPlugin *dataLoggerPlugin);
	~DataLogger_API();

	// PLUGIN RELATED
	Q_INVOKABLE QString showAvailableMonitors();
	Q_INVOKABLE QString showAvailableDevices();
	Q_INVOKABLE QString showMonitorsOfDevice(QString device);
	Q_INVOKABLE QString enableMonitor(QString monitor);
	Q_INVOKABLE QString disableMonitor(QString monitor);
	Q_INVOKABLE void setRunning(bool running);
	Q_INVOKABLE void clearData();

	// TOOL RELATED
	Q_INVOKABLE QString createTool();
	Q_INVOKABLE QString getToolList();
	Q_INVOKABLE QString enableMonitorOfTool(QString toolName, QString monitor);
	Q_INVOKABLE QString disableMonitorOfTool(QString toolName, QString monitor);
	Q_INVOKABLE void setLogPathOfTool(QString toolName, QString path);
	Q_INVOKABLE void logAtPathForTool(QString toolName, QString path);
	Q_INVOKABLE void continuousLogAtPathForTool(QString toolName, QString path);
	Q_INVOKABLE void stopContinuousLogForTool(QString toolName);
	Q_INVOKABLE void importDataFromPathForTool(QString toolName, QString path);

private:
	DataLoggerPlugin *m_dataLoggerPlugin;
	QString m_activeTool;
};
} // namespace scopy::datamonitor
#endif // DATALOGGER_API_H
