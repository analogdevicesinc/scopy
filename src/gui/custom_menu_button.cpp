#include "ui_custom_menu_button.h"

#include "custom_menu_button.hpp"

using namespace adiscope::gui;

CustomMenuButton::CustomMenuButton(QString labelText, bool checkboxVisible, bool checkBoxChecked, QWidget* parent)
	: CustomMenuButton(parent)
{

	m_ui->lblCustomMenuButton->setText(labelText);
	m_ui->checkBoxCustomMenuButton->setVisible(checkboxVisible);

	if (checkboxVisible) {
		checkBoxToggled(checkBoxChecked);
		m_ui->checkBoxCustomMenuButton->setChecked(checkBoxChecked);
	}
}

CustomMenuButton::CustomMenuButton(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::CustomMenuButton)
	, m_floatingMenu(false)
{
	m_ui->setupUi(this);
	connect(m_ui->checkBoxCustomMenuButton, &QCheckBox::toggled, this, &CustomMenuButton::checkBoxToggled);
}

CustomMenuButton::~CustomMenuButton() { delete m_ui; }

void CustomMenuButton::setLabel(const QString& text) { m_ui->lblCustomMenuButton->setText(text); }

void CustomMenuButton::setCheckboxVisible(bool visible) { m_ui->checkBoxCustomMenuButton->setVisible(visible); }

CustomPushButton* CustomMenuButton::getBtn() { return m_ui->btnCustomMenuButton; }

QCheckBox* CustomMenuButton::getCheckBox() { return m_ui->checkBoxCustomMenuButton; }

bool CustomMenuButton::getCheckBoxState() { return getCheckBox()->isChecked(); }

void CustomMenuButton::setCheckBoxState(bool checked) { m_ui->checkBoxCustomMenuButton->setChecked(checked); }

void CustomMenuButton::setMenuFloating(bool floating) { m_floatingMenu = floating; }

void CustomMenuButton::checkBoxToggled(bool toggled)
{
	if (!toggled) {
		m_ui->btnCustomMenuButton->setChecked(false);
	}

	if (!m_floatingMenu) {
		m_ui->btnCustomMenuButton->setEnabled(toggled);
	}
}
