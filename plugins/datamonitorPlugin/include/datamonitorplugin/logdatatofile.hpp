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

	void continuousLogData(QString path);
	void logData(QString path);
	void loadData(QString path);

Q_SIGNALS:
	void startLogData();
	void logDataError();
	void logDataCompleted();
	void startLoadData();
	void loadDataCompleted();

private:
	DataAcquisitionManager *m_dataAcquisitionManager;
	QString *currentFileHeader;
};
} // namespace datamonitor
} // namespace scopy
#endif // LOGDATATOFILE_HPP
