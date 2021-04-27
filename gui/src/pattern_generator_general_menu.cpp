#include "ui_pattern_generator_general_menu.h"

#include <QCheckBox>

#include <scopy/gui/pattern_generator_general_menu.hpp>

using namespace scopy::gui;

constexpr int DIGITAL_NR_CHANNELS = 16;

PatternGeneratorGeneralMenu::PatternGeneratorGeneralMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::PatternGeneratorGeneralMenu)
{
	m_ui->setupUi(this);

	initUi();
}

PatternGeneratorGeneralMenu::~PatternGeneratorGeneralMenu() { delete m_ui; }

void PatternGeneratorGeneralMenu::initUi()
{
	m_ui->widgetMenuHeader->setLabel("General Settings");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#4a64ff"));

	m_ui->widgetAvailableChannelsSubsSep->setLabel("AVAILABEL CHANNELS");
	m_ui->widgetAvailableChannelsSubsSep->setButtonVisible(false);

	for (uint8_t i = 0; i < DIGITAL_NR_CHANNELS; ++i) {
		QCheckBox* channelBox = new QCheckBox("DIO " + QString::number(i));
		m_ui->gridLayoutChannelEnumerator->addWidget(channelBox, i % 8, i / 8);
	}
}
