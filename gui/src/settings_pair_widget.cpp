#include "ui_settings_pair_widget.h"

#include <scopy/gui/settings_pair_widget.hpp>

using namespace scopy::gui;

SettingsPairWidget::SettingsPairWidget(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SettingsPairWidget)
{
	m_ui->setupUi(this);
}

SettingsPairWidget::~SettingsPairWidget() { delete m_ui; }

CustomPushButton* SettingsPairWidget::getGeneralSettingsBtn() { return m_ui->btnGenSettings; }

QPushButton* SettingsPairWidget::getSettingsBtn() { return m_ui->btnSettings; }
