#include <customSwitch.h>
#include <smallOnOffSwitch.h>
#include <widgets/menuonoffswitch.h>

using namespace scopy;

MenuOnOffSwitch::MenuOnOffSwitch(QString title, QWidget *parent, bool medium)
	: QWidget(parent)
{
	auto lay = new QHBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	m_label = new QLabel(title, this);
	if(medium == false) {
		m_switch = new SmallOnOffSwitch(this);
		StyleHelper::MenuOnOffSwitchButton(dynamic_cast<SmallOnOffSwitch *>(m_switch), "menuButton");
		StyleHelper::MenuSmallLabel(m_label, "menuLabel");
	} else {
		m_switch = new CustomSwitch(this);
		StyleHelper::MenuMediumLabel(m_label, "menuLabel");
	}

	lay->addWidget(m_label);
	lay->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
	lay->addWidget(m_switch);
	applyStylesheet();
}

MenuOnOffSwitch::~MenuOnOffSwitch() {}

QAbstractButton *MenuOnOffSwitch::onOffswitch() { return m_switch; }

void MenuOnOffSwitch::applyStylesheet() { StyleHelper::MenuOnOffSwitch(this, "menuOnOffSwitch"); }

#include "moc_menuonoffswitch.cpp"
