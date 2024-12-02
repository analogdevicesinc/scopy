#ifndef DATALOGGERCONTROLLER_H
#define DATALOGGERCONTROLLER_H

#include "dataloggermodel.h"
#include "gui/dataloggerview.hpp"

namespace scopy {
namespace datalogger {

class DataLoggerController : public QObject
{
	Q_OBJECT
public:
	DataLoggerController(bool lastValue, bool average, bool all);
	~DataLoggerController();

	void createChannel(QString name, CHANNEL_DATA_TYPE type);
	void destroyChannel(QString name);
	void resetChannel(QString name);
	void logData();
	QWidget *getWidget();
	bool isDataLoggerOn();
	void setWarningMessage(QString message);

	void setIsRunningOn(bool newIsRunningOn);

	void setUseNativeDialog(bool nativeDialog);

public Q_SLOTS:
	void receiveValue(QString name, QString value);
	void stopLogger();
	void startLogger();
	void dataLoggerToggled(bool toggled);
	void setPath(QString path);
	QString getPath();
	bool isOverwrite();
	void setOverwrite(bool en);
	int getRecordingTimeInterval();
	void setRecodingTimeInterval(int interval);

Q_SIGNALS:
	void toggleDataLogger(bool toggled);
	void isDataLogging(bool isDataLogging);

private:
	DataLoggerModel *dataLoggerModel;
	gui::DataLoggerView *dataLoggerView;
	bool isRunningOn;
	void attemptDataLogging();
};
} // namespace datalogger
} // namespace scopy

#endif // DATALOGGERCONTROLLER_H
