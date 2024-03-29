#ifndef LOGDATATOFILE_HPP
#define LOGDATATOFILE_HPP

#include <QObject>
#include <dataacquisitionmanager.hpp>

namespace scopy {
namespace datamonitor {
class LogDataToFile : public QObject
{
	Q_OBJECT
public:
	explicit LogDataToFile(DataAcquisitionManager *dataAcquisitionManager, QObject *parent = nullptr);

	void logData(QString path);
	void loadData(QString path);

Q_SIGNALS:
	void startLogData();
	void logDataCompleted();
	void startLoadData();
	void loadDataCompleted();

private:
	DataAcquisitionManager *m_dataAcquisitionManager;
	QString dateTimeFormat = "dd/MM/yyyy hh:mm:ss";
};
} // namespace datamonitor
} // namespace scopy
#endif // LOGDATATOFILE_HPP
