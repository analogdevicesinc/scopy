#ifndef DATAMONITORCONTROLLER_HPP
#define DATAMONITORCONTROLLER_HPP


#include "datamonitorview.hpp"

#include <QObject>

namespace scopy {
namespace datamonitor {

class DataMonitorController : public QObject
{
	Q_OBJECT
public:
	explicit DataMonitorController(QObject *parent = nullptr);
	~DataMonitorController();


	DataMonitorView *dataMonitorView() const;

signals:

private:
	DataMonitorView *m_dataMonitorView;

};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORCONTROLLER_HPP
