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

signals:

private:
	DataAcquisitionManager *m_dataAcquisitionManager;
};
} // namespace datamonitor
} // namespace scopy
#endif // LOGDATATOFILE_HPP
