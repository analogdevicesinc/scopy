#include "datamonitorcontroller.hpp"


using namespace scopy;
using namespace datamonitor;

DataMonitorController::DataMonitorController(QObject *parent)
	: QObject{parent}
{

	m_dataMonitorView = new DataMonitorView();

}

DataMonitorController::~DataMonitorController()
{
	delete m_dataMonitorView;
}

DataMonitorView *DataMonitorController::dataMonitorView() const
{
	return m_dataMonitorView;
}

