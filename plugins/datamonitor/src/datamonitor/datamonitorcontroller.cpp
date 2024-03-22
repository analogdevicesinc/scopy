#include "datamonitorcontroller.hpp"
#include "monitorplot.hpp"

using namespace scopy;
using namespace datamonitor;

DataMonitorController::DataMonitorController(DataAcquisitionManager *dataAcquisitionManager, QObject *parent)
	: QObject{parent}
{

	m_dataAcquisitionManager = dataAcquisitionManager;
	m_dataMonitorView = new DataMonitorView();

	m_dataMonitorSettings = new DataMonitorSettings(m_dataMonitorView->monitorPlot());
	m_dataMonitorSettings->init(m_dataMonitorView->getTitle(), StyleHelper::getColor("ScopyBlue"));

	// populate available monitors list in settings
	m_dataMonitorSettings->addMonitorsList(m_dataAcquisitionManager->getDataMonitorMap());

	// plot settings

	connect(m_dataMonitorSettings, &DataMonitorSettings::monitorToggled, m_dataMonitorView->monitorPlot(),
		[=, this](bool toggled, QString monitorName) {
			// toggle monitor active inside data acquisiton manager
			m_dataAcquisitionManager->updateActiveMonitors(toggled, monitorName);

			// handle monitor on plot
			if(toggled) {
				m_dataMonitorView->monitorPlot()->addMonitor(
					m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName));
			} else {
				m_dataMonitorView->monitorPlot()->removeMonitor(monitorName);
			}
		});

	// view settings
	connect(m_dataMonitorSettings, &DataMonitorSettings::togglePlot, m_dataMonitorView,
		&DataMonitorView::togglePlot);

	connect(m_dataMonitorView, &DataMonitorView::titleChanged, m_dataMonitorSettings,
		&DataMonitorSettings::updateTitle);
}

DataMonitorController::~DataMonitorController() {}

DataMonitorView *DataMonitorController::dataMonitorView() const { return m_dataMonitorView; }

DataMonitorSettings *DataMonitorController::dataMonitorSettings() const { return m_dataMonitorSettings; }
