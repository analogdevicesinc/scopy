#include "dynamic_widget.hpp"

#include "ui_voltmeter_menu.h"

#include <scopy/gui/spinbox_a.hpp>
#include <scopy/gui/voltmeter_menu.hpp>

using namespace scopy::gui;

VoltmeterMenu::VoltmeterMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::VoltmeterMenu)
{
	m_ui->setupUi(this);

	m_ui->widgetChannel1MenuHeader->setLabel("Channel 1");
	m_ui->widgetChannel1MenuHeader->setLineColor(new QColor("#ff7200"));

	m_ui->widgetChannel2MenuHeader->setLabel("Channel 2");
	m_ui->widgetChannel2MenuHeader->setLineColor(new QColor("#9013fe"));

	m_ui->widgetPeakHoldSubsSep->setLabel("PEAK HOLD");
	m_ui->widgetPeakHoldSubsSep->setButtonVisible(true);
	m_ui->widgetPeakHoldSubsSep->setButtonChecked(true);

	m_ui->widgetDataLoggingSubsSep->setLabel("DATA LOGGING");
	m_ui->widgetDataLoggingSubsSep->setButtonVisible(true);
	m_ui->widgetDataLoggingSubsSep->setButtonChecked(true);

	connect(m_ui->widgetPeakHoldSubsSep->getButton(), &QPushButton::clicked,
		[=](bool check) { m_ui->widgetPeakHoldContainer->setVisible(check); });

	connect(m_ui->widgetDataLoggingSubsSep->getButton(), &QPushButton::clicked,
		[=](bool check) { m_ui->widgetDataLoggingContainer->setVisible(check); });

	m_dataLoggingTimer =
		new PositionSpinButton({{"s", 1}, {"min", 60}, {"h", 3600}}, tr("Timer"), 0, 3600, true, false, this);

	m_ui->hLayout_2->addWidget(m_dataLoggingTimer);

	m_dataLoggingTimer->setValue(0);
	enableDataLogging(false);
}

VoltmeterMenu::~VoltmeterMenu() { delete m_ui; }

void VoltmeterMenu::enableDataLogging(bool en)
{
	m_ui->gridWidgetLayout_3->setEnabled(en);
	if (!en) {
		DynamicWidget::setDynamicProperty(m_ui->lineEditFilename, "invalid", false);
	}
}
