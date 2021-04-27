#include "ui_logic_pattern_trigger_menu.h"

#include <scopy/gui/logic_pattern_trigger_menu.hpp>

using namespace scopy::gui;

TriggerMenu::TriggerMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::TriggerMenu)
{
	m_ui->setupUi(this);

	initUi();
}

TriggerMenu::~TriggerMenu() { delete m_ui; }

void TriggerMenu::initUi()
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
