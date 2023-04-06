#include "ui_menu_header.h"

#include <QColor>
#include <QIcon>

#include "menu_header.hpp"

using namespace adiscope::gui;

MenuHeader::MenuHeader(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::MenuHeader)
{
	m_ui->setupUi(this);

	this->setEnableBtnVisible(false);
}

MenuHeader::MenuHeader(const QString& label, const QColor* color, bool enableBtnVisible, QWidget* parent)
	: MenuHeader(parent)
{
	this->setLabel(label);
	this->setLineColor(color);
	this->setEnableBtnVisible(enableBtnVisible);
}

MenuHeader::~MenuHeader() { delete m_ui; }

void MenuHeader::setEnabledBtnState(bool state) { m_ui->btnEnabled->setChecked(state); }

void MenuHeader::setLabel(const QString& text) { m_ui->lblTitle->setText(text); }

void MenuHeader::setLineColor(const QColor* color)
{
	m_ui->lineSeparator->setStyleSheet("border: 2px solid " + color->name());
}

void MenuHeader::setEnableBtnVisible(bool visible) { m_ui->btnEnabled->setVisible(visible); }

QPushButton* MenuHeader::getEnableBtn() { return m_ui->btnEnabled; }
