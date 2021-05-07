#include "dynamic_widget.hpp"

#include "ui_cursors_settings.h"

#include <scopy/gui/cursors_settings.hpp>

using namespace scopy::gui;

CursorsSettings::CursorsSettings(GenericMenu* parent)
	: GenericMenu(parent)
	, m_ui(new Ui::CursorsSettings)
{
	m_ui->setupUi(this);

	initUi();

	connect(m_ui->widgetMenuHeader->getEnableBtn(), &QPushButton::toggled,
		[=](bool toggled) { Q_EMIT enableBtnToggled(toggled); });
}

CursorsSettings::~CursorsSettings() { delete m_ui; }

void CursorsSettings::initUi()
{
	m_ui->widgetVerticalSubsSpr->setLabel("VERTICAL");
	m_ui->widgetVerticalSubsSpr->setButtonVisible(false);

	m_ui->widgetHorizontalSubsSpr->setLabel("HORIZONTAL");
	m_ui->widgetHorizontalSubsSpr->setButtonVisible(false);

	m_ui->widgetCursorReadoutsSubsSpr->setLabel("CURSOR READOUTS");
	m_ui->widgetCursorReadoutsSubsSpr->setButtonVisible(false);

	m_ui->widgetMenuHeader->setLabel("Cursors");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetMenuHeader->setEnableBtnVisible(true);

	//	setCursorReadoutsVisible(true);
}

void CursorsSettings::setBtnNormalTrackVisible(bool visible) { m_ui->widgetBtnNormalTrack->setVisible(visible); }

void CursorsSettings::setCursorReadoutsVisible(bool visible) { m_ui->widgetReadouts->setVisible(visible); }

void CursorsSettings::setMenuButton(bool toggled) { m_ui->widgetMenuHeader->setEnabledBtnState(toggled); }

void CursorsSettings::setHorizontalVisible(bool visible)
{
	m_ui->widgetHorizontalSubsSpr->setVisible(visible);
	m_ui->widgetHorizontal->setVisible(visible);
}

void CursorsSettings::setVerticalVisible(bool visible)
{
	m_ui->widgetVerticalSubsSpr->setVisible(visible);
	m_ui->widgetVertical->setVisible(visible);
}
