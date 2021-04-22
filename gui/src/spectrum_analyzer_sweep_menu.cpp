#include "ui_spectrum_analyzer_sweep_menu.h"

#include <scopy/gui/spectrum_analyzer_sweep_menu.hpp>

using namespace scopy::gui;

SpectrumAnalyzerSweepMenu::SpectrumAnalyzerSweepMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SpectrumAnalyzerSweepMenu)
{
	m_ui->setupUi(this);

	initUi();
}

SpectrumAnalyzerSweepMenu::~SpectrumAnalyzerSweepMenu() { delete m_ui; }

void SpectrumAnalyzerSweepMenu::initUi()
{
	// Initialize titles
	m_ui->widgetMenuHeader->setLabel("Sweep");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetAmplitudeSubsSep->setLabel("AMPLITUDE");
	m_ui->widgetAmplitudeSubsSep->setButtonVisible(false);

	m_ui->widgetFrequencySubsSep->setLabel("FREQUENCY");
	m_ui->widgetFrequencySubsSep->setButtonVisible(false);

	// Initialize frequency controls
	m_startStopRange = new StartStopRangeWidget(0);
	m_startStopRange->setStartValue(0);
	m_startStopRange->setStopValue(50e6);

	m_ui->gridSweepControls->addWidget(m_startStopRange, 0, 0, 2, 2);

	// Initialize amplitude controls
	m_unitPerDiv = new PositionSpinButton(
		{
			{" ", 1e0},
		},
		tr("Scale/Div"), 0.0, 0.0, false, false, this);

	m_ui->stackedWidgetDivision->addWidget(m_unitPerDiv);

	m_top = new PositionSpinButton(
		{
			{" ", 1e0},
		},
		tr("Top"), -200.0, 300.0, false, false, this);
	m_topScale = new ScaleSpinButton(
		{{{"pV/√Hz", 1e-12}, {"nV/√Hz", 1e-9}, {"μV/√Hz", 1e-6}, {"mV/√Hz", 1e-3}, {"V/√Hz", 1e0}}}, tr("Top"),
		1e-12, 10e1, false, false, this);
	m_ui->stackedWidgetTop->addWidget(m_top);
	m_ui->stackedWidgetTop->addWidget(m_topScale);

	m_bottom = new PositionSpinButton(
		{
			{" ", 1e0},
		},
		tr("Bottom"), -200.0, 300.0, false, false, this);
	m_bottomScale = new ScaleSpinButton(
		{{{"pV/√Hz", 1e-12}, {"nV/√Hz", 1e-9}, {"μV/√Hz", 1e-6}, {"mV/√Hz", 1e-3}, {"V/√Hz", 1e0}}},
		tr("Bottom"), 1e-12, 10e1, false, false, this);
	m_ui->stackedWidgetBottom->addWidget(m_bottom);
	m_ui->stackedWidgetBottom->addWidget(m_bottomScale);

	// Initialize vertical axis controls
	m_unitPerDiv->setMinValue(1);
	m_unitPerDiv->setMaxValue(200 / 10);

	// Initialize Units ComboBox
	m_magTypes = {
		{tr("dBFS"), FftDisplayPlot::MagnitudeType::DBFS},
		{tr("dBV"), FftDisplayPlot::MagnitudeType::DBV},
		{tr("dBu"), FftDisplayPlot::MagnitudeType::DBU},
		{tr("Vpeak"), FftDisplayPlot::MagnitudeType::VPEAK},
		{tr("Vrms"), FftDisplayPlot::MagnitudeType::VRMS},
		{tr("V/√Hz"), FftDisplayPlot::MagnitudeType::VROOTHZ},
	};

	m_ui->cmbBoxUnits->blockSignals(true);
	m_ui->cmbBoxUnits->clear();

	for (auto it = m_magTypes.begin(); it != m_magTypes.end(); ++it) {
		m_ui->cmbBoxUnits->addItem(it->first);
	}

	m_ui->cmbBoxUnits->blockSignals(false);
}
