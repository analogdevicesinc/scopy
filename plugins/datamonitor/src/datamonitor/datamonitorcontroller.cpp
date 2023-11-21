#include "datamonitorcontroller.hpp"

using namespace scopy;
using namespace datamonitor;

DataMonitorController::DataMonitorController(DataMonitorModel *model, QObject *parent)
	: model(model)
{
	dataMonView = new DataMonitorView(model);

	connect(model, &DataMonitorModel::valueUpdated, dataMonView,
		[=](uint32_t time, double value) { dataMonView->plotData((double)time, value); });
	connect(model, &DataMonitorModel::minValueUpdated, dataMonView, &DataMonitorView::updateMinValue);
	connect(model, &DataMonitorModel::maxValueUpdated, dataMonView, &DataMonitorView::updateMaxValue);
}

DataMonitorController::~DataMonitorController() { delete dataMonView; }

DataMonitorView *DataMonitorController::getDataMonitorView() { return dataMonView; }

DataMonitorModel *DataMonitorController::getModel() const { return model; }

void DataMonitorController::updateValue(double xValue, double yValue) { model->updateValue(xValue, yValue); }
