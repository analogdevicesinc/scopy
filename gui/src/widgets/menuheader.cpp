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

	m_label = new QLabel(title, this);
	m_line = new QFrame(this);
	m_pen = pen;

	lay->addWidget(m_label);
	lay->addWidget(m_line);
	applyStylesheet();
}

MenuHeaderWidget::~MenuHeaderWidget() {}

QLabel *MenuHeaderWidget::label() { return m_label; }

void MenuHeaderWidget::applyStylesheet()
{
	StyleHelper::MenuHeaderLabel(m_label, "menuLabel");
	StyleHelper::MenuHeaderLine(m_line, m_pen, "menuSeparator");
	StyleHelper::MenuHeaderWidget(this, "menuHeader");
}

#include "moc_menuheader.cpp"
