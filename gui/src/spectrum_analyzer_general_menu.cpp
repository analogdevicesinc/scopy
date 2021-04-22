#include "ui_spectrum_analyzer_general_menu.h"

#include <scopy/gui/spectrum_analyzer_general_menu.hpp>

using namespace scopy::gui;

SpectrumAnalyzerGeneralMenu::SpectrumAnalyzerGeneralMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SpectrumAnalyzerGeneralMenu)
{
	m_ui->setupUi(this);

	m_ui->widgetMenuHeader->setLabel("General settings");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#4A64FF"));

	connect(m_ui->btnExport, &QPushButton::clicked, this, &SpectrumAnalyzerGeneralMenu::btnExportClicked);
}

SpectrumAnalyzerGeneralMenu::~SpectrumAnalyzerGeneralMenu() { delete m_ui; }

void SpectrumAnalyzerGeneralMenu::btnExportClicked() {}
