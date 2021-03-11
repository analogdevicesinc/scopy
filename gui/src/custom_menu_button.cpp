#include "ui_custom_menu_button.h"

#include <scopy/gui/custom_menu_button.hpp>

using namespace scopy::gui;

CustomMenuButton::CustomMenuButton(QWidget* parent, QString labelText, bool checkboxVisible)
	: QWidget(parent)
	, m_ui(new Ui::CustomMenuButton)
{
	m_ui->setupUi(this);

	m_ui->lblCustomMenuButton->setText(labelText);
	m_ui->checkBoxCustomMenuButton->setVisible(checkboxVisible);
}

CustomMenuButton::~CustomMenuButton() { delete m_ui; }

CustomPushButton* CustomMenuButton::getButton() { return m_ui->btnCustomMenuButton; }

QLabel* CustomMenuButton::getLabel() { return m_ui->lblCustomMenuButton; }

QCheckBox* CustomMenuButton::getCheckBox() { return m_ui->checkBoxCustomMenuButton; }
