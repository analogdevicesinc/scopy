#include "ui_menu_header.h"

#include <QColor>
#include <QIcon>

#include <scopy/gui/menu_header.hpp>

using namespace scopy::gui;

MenuHeader::MenuHeader(QWidget* parent, const QString& label, const QColor* color)
	: QWidget(parent)
	, m_ui(new Ui::MenuHeader)
{
	m_ui->setupUi(this);

	m_ui->lblTitle->setText(label);
	m_ui->lineSeparator->setStyleSheet("border: 2px solid " + color->name());
}

MenuHeader::~MenuHeader() { delete m_ui; }
