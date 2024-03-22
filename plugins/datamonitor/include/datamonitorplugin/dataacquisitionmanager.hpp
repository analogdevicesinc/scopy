#ifndef DATAACQUISITIONMANAGER_HPP
#define DATAACQUISITIONMANAGER_HPP

#include <QElapsedTimer>
#include <QMap>
#include <QObject>

#include "datamonitormodel.hpp"
#include "scopy-datamonitorplugin_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATAMONITORPLUGIN_EXPORT DataAcquisitionManager : public QObject
{
	Q_OBJECT
public:
	explicit DataAcquisitionManager(QObject *parent = nullptr);
	~DataAcquisitionManager();

	void clearMonitorsData();
	QList<QString> getMonitors();

	// will read data once
	void readData();
	QList<QString> getActiveMonitors();
	void updateActiveMonitors(bool toggled, QString monitorName);
	QMap<QString, DataMonitorModel *> *getDataMonitorMap() const;

signals:
	void requestUpdateActiveMonitors();
	void requestRead();
	void activeMonitorsUpdated();

private:
	QMap<QString, int> *m_activeMonitorsMap;
	QMap<QString, DataMonitorModel *> *m_dataMonitorMap;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAACQUISITIONMANAGER_HPP
