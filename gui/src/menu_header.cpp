#include "ui_menu_header.h"

#include <QColor>
#include <QIcon>

#include <scopy/gui/menu_header.hpp>

using namespace scopy::gui;

MenuHeader::MenuHeader(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::MenuHeader)
{
	m_ui->setupUi(this);
}

MenuHeader::MenuHeader(const QString& label, const QColor* color, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::MenuHeader)
{
	m_ui->setupUi(this);

	this->setLabel(label);
	this->setLineColor(color);
}

MenuHeader::~MenuHeader() { delete m_ui; }

void MenuHeader::setLabel(const QString& text) { m_ui->lblTitle->setText(text); }

void MenuHeader::setLineColor(const QColor* color)
{
	m_ui->lineSeparator->setStyleSheet("border: 2px solid " + color->name());
}
