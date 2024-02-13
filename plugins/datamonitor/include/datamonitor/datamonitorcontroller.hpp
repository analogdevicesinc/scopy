#ifndef DATAMONITORCONTROLLER_HPP
#define DATAMONITORCONTROLLER_HPP

#include "dataacquisitionmanager.hpp"
#include "datamonitorsettings.hpp"
#include "datamonitorview.hpp"

#include <QObject>

namespace scopy {
namespace datamonitor {

class DataMonitorController : public QObject
{
	Q_OBJECT
public:
	explicit DataMonitorController(DataAcquisitionManager *dataAcquisitionManager, QObject *parent = nullptr);
	~DataMonitorController();

	DataMonitorView *dataMonitorView() const;

	DataMonitorSettings *dataMonitorSettings() const;

signals:

private:
	DataMonitorView *m_dataMonitorView;
	DataMonitorSettings *m_dataMonitorSettings;
	DataAcquisitionManager *m_dataAcquisitionManager;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORCONTROLLER_HPP
