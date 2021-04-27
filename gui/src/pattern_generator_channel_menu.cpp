#include "ui_pattern_generator_channel_menu.h"

#include <scopy/gui/pattern_generator_channel_menu.hpp>

using namespace scopy::gui;

PatternGeneratorChannelMenu::PatternGeneratorChannelMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::PatternGeneratorChannelMenu)
{
	m_ui->setupUi(this);

	initUi();
}

PatternGeneratorChannelMenu::~PatternGeneratorChannelMenu() { delete m_ui; }

void PatternGeneratorChannelMenu::initUi()
{
	m_ui->widgetMenuHeader->setLabel("Channel Settings");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#4a64ff"));

	m_ui->widgetGroupSubsSep->setLabel("GROUP");
	m_ui->widgetGroupSubsSep->setButtonVisible(false);

	m_ui->widgetGroup->setVisible(false);
	m_ui->widgetGroupSubsSep->setVisible(false);

	m_ui->btnOutputMode->setDisabled(true);
	m_ui->lineEditName->setDisabled(true);
	m_ui->lineEditTraceHeight->setDisabled(true);
	m_ui->cmbBoxPattern->setDisabled(true);
}
