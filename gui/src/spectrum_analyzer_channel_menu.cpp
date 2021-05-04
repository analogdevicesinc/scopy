#include "ui_spectrum_analyzer_channel_menu.h"

#include <scopy/gui/spectrum_analyzer_channel_menu.hpp>

using namespace scopy::gui;

SpectrumAnalyzerChannelMenu::SpectrumAnalyzerChannelMenu(const QString& menuTitle, QColor* lineColor, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SpectrumAnalyzerChannelMenu)
{
	m_ui->setupUi(this);

	initUi(menuTitle, lineColor);
}

SpectrumAnalyzerChannelMenu::~SpectrumAnalyzerChannelMenu() { delete m_ui; }

void SpectrumAnalyzerChannelMenu::initUi(const QString& menuTitle, QColor* lineColor)
{
	// Temporary: each channel will set its name and line color
	m_ui->widgetMenuHeader->setLabel(menuTitle);
	m_ui->widgetMenuHeader->setLineColor(lineColor);

	// Initialize Type ComboBox
	m_avgTypes = {
		{tr("Sample"), FftDisplayPlot::AverageType::SAMPLE},
		{tr("Peak Hold"), FftDisplayPlot::AverageType::PEAK_HOLD},
		{tr("Peak Hold Continous"), FftDisplayPlot::AverageType::PEAK_HOLD_CONTINUOUS},
		{tr("Min Hold"), FftDisplayPlot::AverageType::MIN_HOLD},
		{tr("Min Hold Continous"), FftDisplayPlot::AverageType::MIN_HOLD_CONTINUOUS},
		{tr("Linear RMS"), FftDisplayPlot::AverageType::LINEAR_RMS},
		{tr("Linear dB"), FftDisplayPlot::AverageType::LINEAR_DB},
		{tr("Exponential RMS"), FftDisplayPlot::AverageType::EXPONENTIAL_RMS},
		{tr("Exponential dB"), FftDisplayPlot::AverageType::EXPONENTIAL_DB},
	};

	m_ui->cmbBoxType->blockSignals(true);
	m_ui->cmbBoxType->clear();

	for (auto it = m_avgTypes.begin(); it != m_avgTypes.end(); ++it) {
		m_ui->cmbBoxType->addItem(it->first);
	}

	m_ui->cmbBoxType->blockSignals(false);

	// Initialize Window ComboBox
	m_winTypes = {
		{tr("Flat top"), FftWinType::FLAT_TOP},
		{tr("Rectangular"), FftWinType::RECTANGULAR},
		{tr("Triangular (Bartlett)"), FftWinType::TRIANGULAR},
		{tr("Hamming"), FftWinType::HAMMING},
		{tr("Hann"), FftWinType::HANN},
		{tr("Blackman-Harris"), FftWinType::BLACKMAN_HARRIS},
		{tr("Kaiser"), FftWinType::KAISER},
	};

	m_ui->cmbBoxWindow->blockSignals(true);
	m_ui->cmbBoxWindow->clear();

	for (auto it = m_winTypes.begin(); it != m_winTypes.end(); ++it) {
		m_ui->cmbBoxWindow->addItem(it->first);
	}

	m_ui->cmbBoxWindow->blockSignals(false);
}
