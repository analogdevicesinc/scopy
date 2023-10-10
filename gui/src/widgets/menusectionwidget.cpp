#include <stylehelper.h>
#include <widgets/menusectionwidget.h>

using namespace scopy;
MenuSectionWidget::MenuSectionWidget(QWidget *parent)
	: QWidget(parent)
{
	m_layout = new QVBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	setLayout(m_layout);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
	StyleHelper::MenuSectionWidget(this, "menuSection");
}

MenuSectionWidget::~MenuSectionWidget() {}

QVBoxLayout *MenuSectionWidget::contentLayout() const { return m_layout; }

MenuVScrollArea::MenuVScrollArea(QWidget *parent)
{
	QWidget *w = new QWidget(this);
	m_layout = new QVBoxLayout(w);
	setWidget(w);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	setLayout(m_layout);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
}

MenuVScrollArea::~MenuVScrollArea() {}

QVBoxLayout *MenuVScrollArea::contentLayout() const { return m_layout; }

#include "moc_menusectionwidget.cpp"
