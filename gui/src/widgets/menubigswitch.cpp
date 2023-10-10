#include "widgets/menubigswitch.h"

using namespace scopy;

MenuBigSwitch::MenuBigSwitch(QString on, QString off, QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QHBoxLayout(this);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	m_switch = new CustomSwitch(this);
	m_switch->setOn(on);
	m_switch->setOff(off);

	lay->addWidget(m_switch);
	applyStylesheet();
}

MenuBigSwitch::~MenuBigSwitch() {}

CustomSwitch *MenuBigSwitch::onOffswitch() { return m_switch; }

void MenuBigSwitch::applyStylesheet()
{
	StyleHelper::MenuBigSwitchButton(m_switch, "menuButton");
	StyleHelper::MenuBigSwitch(this, "menuBigSwitch");
}

#include "moc_menubigswitch.cpp"
