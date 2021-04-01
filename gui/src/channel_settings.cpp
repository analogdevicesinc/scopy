#include "ui_channel_settings.h"

#include <scopy/gui/channel_settings.hpp>

using namespace scopy::gui;

ChannelSettings::ChannelSettings(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ChannelSettings)
{
	m_ui->setupUi(this);

	m_ui->widgetMenuHeader->setLabel("Channel");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#ff7200"));

	m_ui->widgetSubsectionSeparatorMath->setLabel("MATH");
	m_ui->widgetSubsectionSeparatorMath->setButtonVisible(false);

	m_ui->widgetSubsectionSeparatorSettings->setLabel("SETTINGS");
	m_ui->widgetSubsectionSeparatorSettings->setButtonVisible(false);

	m_ui->widgetSubsectionSeparatorVertical->setLabel("VERTICAL");
	m_ui->widgetSubsectionSeparatorVertical->setButtonVisible(false);

	m_ui->widgetSubsectionSeparatorHorizontal->setLabel("HORIZONTAL");
	m_ui->widgetSubsectionSeparatorHorizontal->setButtonVisible(false);
}

ChannelSettings::~ChannelSettings() { delete m_ui; }

void ChannelSettings::setMathLayoutVisibility(bool visible) { m_ui->widgetMathSetting->setVisible(visible); }

void ChannelSettings::setFilter1Visibility(bool visible) { m_ui->widgetFilter1->setVisible(visible); }

void ChannelSettings::setFilter2Visibility(bool visible) { m_ui->widgetFilter2->setVisible(visible); }
