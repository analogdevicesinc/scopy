#ifndef DATALOGGERVIEW_H
#define DATALOGGERVIEW_H

#include <spinbox_a.hpp>
#include <QWidget>
#include <QLineEdit>
#include <QRadioButton>
#include <customSwitch.h>
#include <QVBoxLayout>
#include <QFileDialog>
#include <qcombobox.h>
#include <QLabel>

namespace scopy {
class DataLoggerView : public QWidget
{
	Q_OBJECT

public:
	explicit DataLoggerView(bool lastValue, bool average, bool all, QWidget *parent = nullptr);
	~DataLoggerView();

	void init();
	void chooseFile();
	QWidget* getDataLoggerViewWidget();
	bool isDataLoggingOn();
	bool isOverwrite();
	QString getFilter();
	void toggleDataLoggerSwitch(bool toggle);
	void setDataLoggerPath(QString path);
	QString getDataLoggerPath();
	void setOverwrite(bool en);
	int getTimerInterval();
	void setTimerInterval(int interval);
	void isDataLoggerRunning(bool en);

	void setWarningMessage(QString message);
	void enableDataLoggerFields(bool en);
	void disableDataLogging(bool en);

	void setUseNativeDialog(bool nativeDialog);
	bool useNativeDialog;

private:
	PositionSpinButton *data_logging_timer;
	QWidget *dataLoggingWidget;
	QVBoxLayout *dataLoggingLayout;
	QLineEdit *dataLoggingFilePath;
	QRadioButton *overwriteRadio;
	QRadioButton *appendRadio;
	CustomSwitch *dataLoggingSwitch;
	QComboBox *dataLoggerFilter;
	QString filename;
	QLabel *warningMessage;
	QPushButton *dataLoggingBrowseBtn;

Q_SIGNALS:
	void toggleDataLogger(bool toggled);
	void pathChanged(QString path);
	void timeIntervalChanged(double inetrval);

};
}
#endif // DATALOGGERVIEW_H
