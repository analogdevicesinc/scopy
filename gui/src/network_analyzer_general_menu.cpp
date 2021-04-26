#include "ui_network_analyzer_general_menu.h"

#include <scopy/gui/network_analyzer_general_menu.hpp>

using namespace scopy::gui;

NetworkAnalyzerGeneralMenu::NetworkAnalyzerGeneralMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::NetworkAnalyzerGeneralMenu)
{
	m_ui->setupUi(this);

	initUi();
}

NetworkAnalyzerGeneralMenu::~NetworkAnalyzerGeneralMenu() { delete m_ui; }

void NetworkAnalyzerGeneralMenu::initUi()
{
	m_ui->widgetMenuHeader->setLabel("General Settings");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetPlotSubsSep->setLabel("PLOT");
	m_ui->widgetPlotSubsSep->setButtonVisible(false);

	m_ui->widgetExportSubsSep->setLabel("EXPORT");
	m_ui->widgetExportSubsSep->setButtonVisible(false);

	m_ui->widgetReferenceSubsSep->setLabel("REFRENCE");
	m_ui->widgetReferenceSubsSep->setButtonVisible(false);
}
