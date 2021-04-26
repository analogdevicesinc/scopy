#include "ui_logic_analyzer_trigger_menu.h"

#include <scopy/gui/logic_analyzer_trigger_menu.hpp>

using namespace scopy::gui;

LogicAnalyzerTriggerMenu::LogicAnalyzerTriggerMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::LogicAnalyzerTriggerMenu)
{
	m_ui->setupUi(this);

	// TODO: remove this and find a way to resize a single menu
//	parent->setMinimumWidth(375);

	initUi();
}

LogicAnalyzerTriggerMenu::~LogicAnalyzerTriggerMenu() { delete m_ui; }

void LogicAnalyzerTriggerMenu::initUi()
{
	m_ui->widgetMenuHeader->setLabel("Trigger Settings");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#4a64ff"));

	m_ui->widgetExternalTriggerSubsSep->setLabel("EXTERNAL TRIGGER");
	m_ui->widgetExternalTriggerSubsSep->setButtonVisible(false);

	m_ui->lblWarningFw->setVisible(false);

	m_ui->cmbBoxExternalTriggerCondition->setDisabled(true);
	m_ui->cmbBoxExternalTriggerSource->setDisabled(true);

	m_ui->cmbBoxTriggerLogic->addItem("OR");
	m_ui->cmbBoxTriggerLogic->addItem("AND");
}
