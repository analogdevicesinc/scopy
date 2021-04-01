#include <scopy/gui/network_analyzer_menu.hpp>
#include "ui_network_analyzer_menu.h"

using namespace scopy::gui;

NetworkAnalyzerMenu::NetworkAnalyzerMenu(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::NetworkAnalyzerMenu)
{
	m_ui->setupUi(this);

	m_ui->widgetCursorsMenuHeader->setLabel("Cursors");
	m_ui->widgetCursorsMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetSettingsMenuHeader->setLabel("Settings");
	m_ui->widgetSettingsMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetGeneralSettingsMenuHeader->setLabel("General Settings");
	m_ui->widgetGeneralSettingsMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetPlotSubsSep->setLabel("PLOT");
	m_ui->widgetPlotSubsSep->setButtonVisible(false);

	m_ui->widgetSweepSubsSep->setLabel("SWEEP");
	m_ui->widgetSweepSubsSep->setButtonVisible(false);

	m_ui->widgetExportSubsSep->setLabel("EXPORT");
	m_ui->widgetExportSubsSep->setButtonVisible(false);

	m_ui->widgetDisplaySubsSep->setLabel("DISPLAY");
	m_ui->widgetDisplaySubsSep->setButtonVisible(false);

	m_ui->widgetResponseSubsSep->setLabel("RESPONSE");
	m_ui->widgetResponseSubsSep->setButtonVisible(false);

	m_ui->widgetReferenceSubsSep->setLabel("REFRENCE");
	m_ui->widgetReferenceSubsSep->setButtonVisible(false);

	m_ui->widgetReferenceSubsSep_2->setLabel("REFRENCE");
	m_ui->widgetReferenceSubsSep_2->setButtonVisible(false);

	m_ui->widgetCursorReadoutsSubsSep->setLabel("CURSOR READOUTS");
	m_ui->widgetCursorReadoutsSubsSep->setButtonVisible(false);

	m_ui->widgetBufferPreviewerSubsSep->setLabel("BUFFER PREVIEWER");
	m_ui->widgetBufferPreviewerSubsSep->setButtonVisible(false);
}

NetworkAnalyzerMenu::~NetworkAnalyzerMenu()
{
	delete m_ui;
}
