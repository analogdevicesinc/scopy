#include "qlineedit.h"
#include <stylehelper.h>
#include <widgets/menuheader.h>

using namespace scopy;

MenuHeaderWidget::MenuHeaderWidget(QString title, QPen pen, QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QVBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(3);
	//		lay->setContentsMargins(3,3,3,3);

	m_title = new QLineEdit(title, this);
	m_title->setEnabled(false);
	m_title->setReadOnly(false);

	m_line = new QFrame(this);
	m_pen = pen;

	lay->addWidget(m_title);
	lay->addWidget(m_line);
	applyStylesheet();
}

MenuHeaderWidget::~MenuHeaderWidget() {}

QLineEdit *MenuHeaderWidget::title() { return m_title; }

void MenuHeaderWidget::applyStylesheet()
{
	StyleHelper::MenuCollapseHeaderLineEdit(m_title, "menuLabel");
	StyleHelper::MenuHeaderLine(m_line, m_pen, "menuSeparator");
	StyleHelper::MenuHeaderWidget(this, "menuHeader");
}

#include "moc_menuheader.cpp"
