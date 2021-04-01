#include "ui_pattern_generator_menu.h"

#include <scopy/gui/pattern_generator_menu.hpp>

using namespace scopy::gui;

PatternGeneratorMenu::PatternGeneratorMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::PatternGeneratorMenu)
{
	m_ui->setupUi(this);

	m_ui->widgetChannelSettingsMenuHeader->setLabel("Channel Settings");
	m_ui->widgetChannelSettingsMenuHeader->setLineColor(new QColor("#4a64ff"));

	m_ui->widgetGeneralSettingsMenuHeader->setLabel("General Settings");
	m_ui->widgetGeneralSettingsMenuHeader->setLineColor(new QColor("#4a64ff"));

	m_ui->widgetTriggerSettingsMenuHeader->setLabel("Trigger Settings");
	m_ui->widgetTriggerSettingsMenuHeader->setLineColor(new QColor("#4a64ff"));

	m_ui->widgetExternalTriggerSubsSep->setLabel("EXTERNAL TRIGGER");
	m_ui->widgetExternalTriggerSubsSep->setButtonVisible(false);

	m_ui->widgetAvailableChannelsSubsSep->setLabel("AVAILABEL CHANNELS");
	m_ui->widgetAvailableChannelsSubsSep->setButtonVisible(false);

	m_ui->widgetGroupSubsSep->setLabel("GROUP");
	m_ui->widgetGroupSubsSep->setButtonVisible(false);
}

PatternGeneratorMenu::~PatternGeneratorMenu() { delete m_ui; }
