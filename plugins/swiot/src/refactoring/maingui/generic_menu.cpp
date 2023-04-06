#include <QVBoxLayout>

#include "generic_menu.hpp"

using namespace adiscope::gui;

GenericMenu::GenericMenu(QWidget* parent)
	: QWidget(parent)
	, m_menuHeader(new MenuHeader(parent))
	, m_menu(new BaseMenu(parent))
	, m_lastOpenPosition(0)
{}

GenericMenu::~GenericMenu()
{
	delete m_menuHeader;
	delete m_menu;
}

void GenericMenu::initInteractiveMenu()
{
	this->setStyleSheet(".QWidget { background-color: none; }");

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(10);
	layout->setContentsMargins(18, 20, 18, 9);

	layout->addWidget(m_menuHeader);
	layout->addWidget(m_menu);

	this->setLayout(layout);
}

void GenericMenu::setMenuHeader(const QString& title, const QColor* lineColor, bool hasEnableBtn)
{
	m_menuHeader->setLabel(title);
	m_menuHeader->setLineColor(lineColor);
	m_menuHeader->setEnabledBtnState(hasEnableBtn);
}

void GenericMenu::insertSection(SubsectionSeparator* section)
{
	BaseMenuItem* item = new BaseMenuItem(m_menu);
	item->setWidget(section);

	m_menu->insertMenuItem(item, m_lastOpenPosition);

	m_lastOpenPosition++;
}

