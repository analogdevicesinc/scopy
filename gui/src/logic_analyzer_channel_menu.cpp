#include "ui_logic_analyzer_channel_menu.h"

#include <scopy/gui/logic_analyzer_channel_menu.hpp>

using namespace scopy::gui;

LogicAnalyzerChannelMenu::LogicAnalyzerChannelMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::LogicAnalyzerChannelMenu)
{
	m_ui->setupUi(this);

	initUi();
}

LogicAnalyzerChannelMenu::~LogicAnalyzerChannelMenu() { delete m_ui; }

void LogicAnalyzerChannelMenu::initUi()
{
	m_ui->widgetMenuHeader->setLabel("Channel Settings");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#4a64ff"));

	m_ui->widgetGroupSubsSep->setLabel("GROUP");
	m_ui->widgetGroupSubsSep->setButtonVisible(false);

	m_ui->lineEditName->setDisabled(true);
	m_ui->lineEditTraceHeight->setDisabled(true);
	m_ui->cmbBoxTrigger->setDisabled(true);

	m_ui->widgetGroupSubsSep->setVisible(false);
	m_ui->widgetStackDecoder->setVisible(false);
}
