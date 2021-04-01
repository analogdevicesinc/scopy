#include "ui_power_supply_menu.h"

#include <scopy/gui/power_supply_menu.hpp>
#include <scopy/gui/spinbox_a.hpp>

using namespace scopy::gui;

PowerSupplyMenu::PowerSupplyMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::PowerSupplyMenu)
{
	m_ui->setupUi(this);

	m_ui->widgetTrakingRationControlMenuHeader->setLabel("Traking ratio control");
	m_ui->widgetTrakingRationControlMenuHeader->setLineColor(new QColor("#4a64ff"));

	m_ui->widgetPositiveOutputMenuHeader->setLabel("Positive output");
	m_ui->widgetPositiveOutputMenuHeader->setLineColor(new QColor("#ff7200"));

	m_ui->widgetNegativeOutputMenuHeader->setLabel("Negative output");
	m_ui->widgetNegativeOutputMenuHeader->setLineColor(new QColor("#9013fe"));

	m_ui->btnSync->click();

	m_valuePos =
		new PositionSpinButton({{tr("mVolts"), 1e-3}, {tr("Volts"), 1e0}}, tr("Value"), 0, 5, true, true, this);

	m_valueNeg = new PositionSpinButton({{tr("mVolts"), 1e-3}, {tr("Volts"), 1e0}}, tr("Value"), -5, 0, true, true,
					    this);

	m_ui->vLayoutValuePos->addWidget(m_valuePos);
	m_ui->vLayoutValueNeg->addWidget(m_valueNeg);
}

PowerSupplyMenu::~PowerSupplyMenu() { delete m_ui; }
