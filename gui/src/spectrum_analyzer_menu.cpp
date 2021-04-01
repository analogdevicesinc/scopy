#include "ui_spectrum_analyzer_menu.h"

#include <scopy/gui/spectrum_analyzer_menu.hpp>

using namespace scopy::gui;

SpectrumAnalyzerMenu::SpectrumAnalyzerMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SpectrumAnalyzerMenu)
{
	m_ui->setupUi(this);

	m_ui->widgetSweepMenuHeader->setLabel("Sweep");
	m_ui->widgetSweepMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetChannelMenuHeader->setLabel("Channel");
	m_ui->widgetChannelMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetGeneralSettingsMenuHeader->setLabel("General settings");
	m_ui->widgetGeneralSettingsMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetImportReferenceChannelsMenuHeader->setLabel("Import Reference Channels");
	m_ui->widgetImportReferenceChannelsMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetMarkerSubsSep->setLabel("MARKER");
	m_ui->widgetMarkerSubsSep->setButtonVisible(false);

	m_ui->widgetGeneralSubsSep->setLabel("GENERAL");
	m_ui->widgetGeneralSubsSep->setButtonVisible(false);

	m_ui->widgetAmplitudeSubsSep->setLabel("AMPLITUDE");
	m_ui->widgetAmplitudeSubsSep->setButtonVisible(false);

	m_ui->widgetFrequencySubsSep->setLabel("FREQUENCY");
	m_ui->widgetFrequencySubsSep->setButtonVisible(false);
}

SpectrumAnalyzerMenu::~SpectrumAnalyzerMenu() { delete m_ui; }
