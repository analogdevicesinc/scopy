#include "ui_spectrum_analyzer_add_reference_menu.h"

#include <scopy/gui/spectrum_analyzer_add_reference_menu.hpp>

using namespace scopy::gui;

SpectrumAnalyzerAddReferenceMenu::SpectrumAnalyzerAddReferenceMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SpectrumAnalyzerAddReferenceMenu)
{
	m_ui->setupUi(this);

	m_ui->widgetMenuHeader->setLabel("Import Reference Channels");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#4A64FF"));
}

SpectrumAnalyzerAddReferenceMenu::~SpectrumAnalyzerAddReferenceMenu() { delete m_ui; }
