#ifndef DATAMONITORCONTROLLER_H
#define DATAMONITORCONTROLLER_H

#include <QObject>

#include "datamonitormodel.hpp"
#include "datamonitorview.hpp"

namespace scopy {
namespace datamonitor {

class DataMonitorController : public QObject
{
	Q_OBJECT
public:
	explicit DataMonitorController(DataMonitorModel *model, QObject *parent = nullptr);
	~DataMonitorController();

	DataMonitorView *getDataMonitorView();
	DataMonitorModel *getModel() const;
	void updateValue(double xValue, double yValue);

private:
	DataMonitorView *dataMonView;
	DataMonitorModel *model;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORCONTROLLER_H
