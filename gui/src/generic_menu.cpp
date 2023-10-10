#include "generic_menu.hpp"

#include <QVBoxLayout>

using namespace scopy::gui;

GenericMenu::GenericMenu(QWidget *parent)
	: QWidget(parent)
	, m_menuHeader(new MenuHeader(parent))
	, m_menu(new BaseMenu(parent))
	, m_lastOpenPosition(0)
{}

GenericMenu::~GenericMenu()
{
	//	delete m_menuHeader;
	//	delete m_menu;
}

void GenericMenu::initInteractiveMenu()
{
	this->setStyleSheet(".QWidget { background-color: none; }");

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSpacing(10);
	layout->setContentsMargins(18, 20, 18, 9);

	layout->addWidget(m_menuHeader);
	layout->addWidget(m_menu);

	this->setLayout(layout);
}

void GenericMenu::setMenuHeader(const QString &title, const QColor *lineColor, bool hasEnableBtn)
{
	m_menuHeader->setLabel(title);
	m_menuHeader->setLineColor(lineColor);
	m_menuHeader->setEnabledBtnState(hasEnableBtn);
}

void GenericMenu::insertSection(SubsectionSeparator *section)
{
	BaseMenuItem *item = new BaseMenuItem(m_menu);
	item->setWidget(section);

	m_menu->insertMenuItem(item, m_lastOpenPosition);

	m_lastOpenPosition++;
}

void GenericMenu::setMenuWidget(QWidget *widget)
{
	this->setStyleSheet(".QWidget { background-color: transparent; }");

	delete m_menuHeader;
	delete m_menu;

	this->layout()->removeWidget(m_menuHeader);
	this->layout()->removeWidget(m_menu);
	this->layout()->setSpacing(0);
	this->layout()->setContentsMargins(0, 0, 0, 0);

	this->layout()->addWidget(widget);
}

void GenericMenu::hideEvent(QHideEvent *) { Q_EMIT menuVisibilityChanged(false); }

void GenericMenu::showEvent(QShowEvent *) { Q_EMIT menuVisibilityChanged(true); }

void GenericMenu::addNewHeaderWidget(QWidget *widget) { m_menuHeader->addNewHeaderWidget(widget); }

#include "moc_generic_menu.cpp"
