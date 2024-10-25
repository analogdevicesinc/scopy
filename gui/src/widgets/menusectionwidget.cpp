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

MenuSectionCollapseWidget::MenuSectionCollapseWidget(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style,
						     MenuCollapseSection::MenuHeaderWidgetType headerType,
						     QWidget *parent)
	: QWidget(parent)
{
	m_layout = new QVBoxLayout(this);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
	setLayout(m_layout);
	m_section = new MenuSectionWidget(parent);
	m_collapse = new MenuCollapseSection(title, style, headerType, m_section);
	m_layout->addWidget(m_section);
	m_section->contentLayout()->addWidget(m_collapse);
}

MenuSectionCollapseWidget::~MenuSectionCollapseWidget() {}

QVBoxLayout *MenuSectionCollapseWidget::contentLayout() const { return m_collapse->contentLayout(); }

void MenuSectionCollapseWidget::add(QWidget *w) { m_collapse->contentLayout()->addWidget(w); }

void MenuSectionCollapseWidget::remove(QWidget *w) { m_collapse->contentLayout()->removeWidget(w); }

bool MenuSectionCollapseWidget::collapsed() { return !m_collapse->header()->isChecked(); }

void MenuSectionCollapseWidget::setCollapsed(bool b) { m_collapse->header()->setChecked(!b); }

MenuCollapseSection *MenuSectionCollapseWidget::collapseSection() { return m_collapse; }
#include "moc_menusectionwidget.cpp"
