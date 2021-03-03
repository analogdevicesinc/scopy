#include "ui_settings_pair_widget.h"

#include <scopy/gui/settings_pair_widget.hpp>

using namespace scopy::gui;

SettingsPairWidget::SettingsPairWidget(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SettingsPairWidget)
	, m_buttonGroup(new QButtonGroup)
{
	m_ui->setupUi(this);

	m_buttonGroup->addButton(m_ui->btnSettings);
	m_buttonGroup->addButton(m_ui->btnGenSettings);
}

SettingsPairWidget::~SettingsPairWidget() { delete m_ui; }
