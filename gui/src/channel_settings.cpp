#include "ui_channel_settings.h"

#include <scopy/gui/channel_settings.hpp>

using namespace scopy::gui;

ChannelSettings::ChannelSettings(GenericMenu* parent)
	: GenericMenu(parent)
	, m_ui(new Ui::ChannelSettings)
{
	m_ui->setupUi(this);

	this->initUi();

	connect(m_ui->widgetMenuHeader->getEnableBtn(), &QPushButton::toggled,
		[=](bool toggled) { Q_EMIT enableBtnToggled(toggled); });
}

ChannelSettings::ChannelSettings(const QString& menuTitle, const QColor* lineColor, GenericMenu* parent)
	: ChannelSettings(parent)
{
	m_ui->widgetMenuHeader->setLabel(menuTitle);
	m_ui->widgetMenuHeader->setLineColor(lineColor);
}

ChannelSettings::~ChannelSettings() { delete m_ui; }

void ChannelSettings::initUi()
{
	m_ui->widgetMenuHeader->setEnableBtnVisible(true);

	m_ui->widgetSubsectionSeparatorMath->setLabel("MATH");
	m_ui->widgetSubsectionSeparatorMath->setButtonVisible(false);

	m_ui->widgetSubsectionSeparatorSettings->setLabel("SETTINGS");
	m_ui->widgetSubsectionSeparatorSettings->setButtonVisible(false);

	m_ui->widgetSubsectionSeparatorVertical->setLabel("VERTICAL");
	m_ui->widgetSubsectionSeparatorVertical->setButtonVisible(false);

	m_ui->widgetSubsectionSeparatorHorizontal->setLabel("HORIZONTAL");
	m_ui->widgetSubsectionSeparatorHorizontal->setButtonVisible(false);
}

void ChannelSettings::setMenuButton(bool toggled) { m_ui->widgetMenuHeader->setEnabledBtnState(toggled); }

void ChannelSettings::setMathLayoutVisibility(bool visible) { m_ui->widgetMathSetting->setVisible(visible); }

void ChannelSettings::setFilter1Visibility(bool visible) { m_ui->widgetFilter1->setVisible(visible); }

void ChannelSettings::setFilter2Visibility(bool visible) { m_ui->widgetFilter2->setVisible(visible); }
