#ifndef DATAMONITORTOOLSETTINGS_HPP
#define DATAMONITORTOOLSETTINGS_HPP

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

namespace scopy {
namespace datamonitor {

class DataMonitorToolSettings : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit DataMonitorToolSettings(QWidget *parent = nullptr);

Q_SIGNALS:
	void readIntervalChanged(double interval);
	void pathChanged(QString path);
	void requestDataLogging(QString path);

private:
	QString filename;
	void chooseFile();
	QLineEdit *dataLoggingFilePath;
	QPushButton *dataLoggingBrowseBtn;
	QPushButton *dataLoggingBtn;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORTOOLSETTINGS_HPP
