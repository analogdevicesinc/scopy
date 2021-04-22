#include "ui_spectrum_analyzer_markers_menu.h"

#include <scopy/gui/spectrum_analyzer_markers_menu.hpp>

using namespace scopy::gui;

SpectrumAnalyzerMarkersMenu::SpectrumAnalyzerMarkersMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SpectrumAnalyzerMarkersMenu)
	, m_markerSelector(new DbClickButtons(this))
{
	m_ui->setupUi(this);

	initUi();
}

SpectrumAnalyzerMarkersMenu::~SpectrumAnalyzerMarkersMenu() { delete m_ui; }

void SpectrumAnalyzerMarkersMenu::initUi()
{
	m_markerTypes = {
		{tr("Manual"), FftDisplayPlot::MarkerType::MANUAL},
		{tr("Peak"), FftDisplayPlot::MarkerType::PEAK},
		{tr("Delta"), FftDisplayPlot::MarkerType::DELTA},
		{tr("Fixed"), FftDisplayPlot::MarkerType::FIXED},
	};

	m_ui->widgetMarkerSubsSep->setLabel("MARKER");
	m_ui->widgetMarkerSubsSep->setButtonVisible(false);

	m_ui->widgetGeneralSubsSep->setLabel("GENERAL");
	m_ui->widgetGeneralSubsSep->setButtonVisible(false);

	// Temporarily disable the delta marker button
	m_ui->btn_4->hide();
	m_ui->btn_3->hide();

	m_markerFreqPos = new PositionSpinButton({{tr("Hz"), 1e0}, {tr("kHz"), 1e3}, {tr("MHz"), 1e6}},
						 tr("Frequency Position"), 0.0, 5e7, true, false, this);
	m_ui->vLayoutMarkerFreqPosLayout->addWidget(m_markerFreqPos);
	m_markerFreqPos->setFineModeAvailable(false);

	// Initialize Marker controls
	m_ui->hLayoutMarkerSelector->addWidget(m_markerSelector);
}
