#include "datamonitorcontroller.hpp"
#include "monitorplot.hpp"

using namespace scopy;
using namespace datamonitor;

DataMonitorController::DataMonitorController(DataAcquisitionManager *dataAcquisitionManager, QObject *parent)
	: QObject{parent}
{

	m_dataAcquisitionManager = dataAcquisitionManager;
	m_dataMonitorView = new DataMonitorView();

	m_dataMonitorSettings = new DataMonitorSettings();
	m_dataMonitorSettings->init(m_dataMonitorView->getTitle(), StyleHelper::getColor("ScopyBlue"));

	// populate available monitors list in settings
	m_dataMonitorSettings->addMonitorsList(m_dataAcquisitionManager->getDataMonitorMap()->keys());

	// plot settings
	connect(m_dataMonitorSettings, &DataMonitorSettings::plotYAxisAutoscaleToggled,
		m_dataMonitorView->monitorPlot(), &MonitorPlot::plotYAxisAutoscaleToggled);
	connect(m_dataMonitorSettings, &DataMonitorSettings::plotYAxisMinValueChange, m_dataMonitorView->monitorPlot(),
		&MonitorPlot::updateYAxisIntervalMin);
	connect(m_dataMonitorSettings, &DataMonitorSettings::plotYAxisMaxValueChange, m_dataMonitorView->monitorPlot(),
		&MonitorPlot::updateYAxisIntervalMax);

	connect(m_dataMonitorSettings, &DataMonitorSettings::changeTimePeriod, m_dataMonitorView->monitorPlot(),
		&MonitorPlot::updateXAxis);

	connect(m_dataMonitorSettings, &DataMonitorSettings::lineStyleIndexChanged, m_dataMonitorView->monitorPlot(),
		&MonitorPlot::changeCurveStyle);

	connect(m_dataMonitorSettings, &DataMonitorSettings::monitorToggled, m_dataMonitorView->monitorPlot(),
		[=](bool toggled, QString monitorName) {
			// toggle monitor active inside data acquisiton manager
			m_dataAcquisitionManager->updateActiveMonitors(toggled, monitorName);

			// handle monitor on plot
			if(m_dataMonitorView->monitorPlot()->hasMonitor(monitorName)) {
				m_dataMonitorView->monitorPlot()->toggleMonitor(toggled, monitorName);

			} else {
				m_dataMonitorView->monitorPlot()->addMonitor(
					m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName));
			}
		});

	// view settings
	connect(m_dataMonitorSettings, &DataMonitorSettings::togglePlot, m_dataMonitorView,
		&DataMonitorView::togglePlot);
	connect(m_dataMonitorSettings, &DataMonitorSettings::togglePeakHolder, m_dataMonitorView,
		&DataMonitorView::togglePeakHolder);
	connect(m_dataMonitorView, &DataMonitorView::titleChanged, m_dataMonitorSettings,
		&DataMonitorSettings::updateTitle);

	// devide in 2 connects one for plot one for view
	connect(m_dataMonitorSettings, &DataMonitorSettings::mainMonitorChanged, m_dataMonitorView,
		[=](QString monitorName) {
			// TODO disconnect old monitor connect to new monitor

			m_dataMonitorView->configureMonitor(
				m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName));

			disconnect(m_dataMonitorSettings, &DataMonitorSettings::resetPeakHolder,
				   m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName),
				   &DataMonitorModel::resetMinMax);

			connect(m_dataMonitorSettings, &DataMonitorSettings::resetPeakHolder,
				m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName),
				&DataMonitorModel::resetMinMax);
		});
}

DataMonitorController::~DataMonitorController() {}

DataMonitorView *DataMonitorController::dataMonitorView() const { return m_dataMonitorView; }

DataMonitorSettings *DataMonitorController::dataMonitorSettings() const { return m_dataMonitorSettings; }
