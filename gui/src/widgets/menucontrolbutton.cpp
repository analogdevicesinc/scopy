#include <dynamicWidget.h>
#include <pluginbase/preferences.h>
#include <stylehelper.h>
#include <widgets/menucontrolbutton.h>

using namespace scopy;

MenuControlButton::MenuControlButton(QWidget *parent)
	: QAbstractButton(parent)
{
	lay = new QHBoxLayout(this);
	lay->setMargin(16);
	lay->setSpacing(16);

	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	setCheckable(true);
	setLayout(lay);

	m_chk = new QCheckBox("", this);
	m_label = new QLabel("", this);
	m_btn = new QPushButton("", this);
	m_color = StyleHelper::getColor("ScopyBlue");
	m_cs = CS_SQUARE;

	lay->addWidget(m_chk);
	lay->addWidget(m_label);
	lay->addWidget(m_btn);
	applyStylesheet();

	connect(this, &QAbstractButton::toggled, this,
		[=](bool b) { setDynamicProperty(this, "selected", b); }); // Hackish - QStyle should be implemented

	dblClickToOpenMenu = QMetaObject::Connection();
	openMenuChecksThis = QMetaObject::Connection();
}

MenuControlButton::~MenuControlButton() {}

void MenuControlButton::setColor(QColor c)
{
	m_color = c;
	applyStylesheet();
}

void MenuControlButton::setCheckBoxStyle(CheckboxStyle cs)
{
	m_cs = cs;
	applyStylesheet();
}

void MenuControlButton::setName(QString s) { m_label->setText(s); }

void MenuControlButton::setDoubleClickToOpenMenu(bool b)
{
	if(b) {
		dblClickToOpenMenu = connect(this, &MenuControlButton::doubleClicked, this, [=]() {
			setChecked(true);
			if(m_btn->isVisible()) {
				m_btn->toggle();
			}
		});
	} else {
		disconnect(dblClickToOpenMenu);
	}
}

void MenuControlButton::setOpenMenuChecksThis(bool b)
{
	if(b) {
		openMenuChecksThis = connect(m_btn, &QAbstractButton::toggled, this, [=](bool b) {
			if(b)
				setChecked(true);
		});
	} else {
		disconnect(openMenuChecksThis);
	}
}

void MenuControlButton::mouseDoubleClickEvent(QMouseEvent *e)
{
	if(e->button() == Qt::LeftButton) {
		if(Preferences::get("general_doubleclick_ctrl_opens_menu").toBool() == true) {
			Q_EMIT doubleClicked();
			return;
		}
	}
	QAbstractButton::mouseDoubleClickEvent(e);
}

void MenuControlButton::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == Qt::LeftButton) {
		if(Preferences::get("general_doubleclick_ctrl_opens_menu").toBool() == true) {
			if(m_btn->isChecked()) {
				m_btn->setChecked(false);
				return;
			}
		}
	}
	QAbstractButton::mousePressEvent(e);
}

QCheckBox *MenuControlButton::checkBox() { return m_chk; }

QPushButton *MenuControlButton::button() { return m_btn; }

void MenuControlButton::applyStylesheet()
{

	StyleHelper::MenuControlWidget(this, m_color, "controlButton");
	switch(m_cs) {
	case CS_CIRCLE:
		StyleHelper::ColoredCircleCheckbox(m_chk, m_color, "chk");
		break;
	case CS_SQUARE:
		StyleHelper::ColoredSquareCheckbox(m_chk, 0xFFFFFF, "chk");
		break;
	case CS_COLLAPSE:
		StyleHelper::CollapseCheckbox(m_chk, "chk");
		break;
	default:
		StyleHelper::BlueSquareCheckbox(m_chk, "chk");
		break;
	}
	StyleHelper::MenuControlLabel(m_label, "name");
	StyleHelper::MenuControlButton(m_btn, "btn");
}

CollapsableMenuControlButton::CollapsableMenuControlButton(QWidget *parent)
	: QWidget(parent)
{
	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	setLayout(m_lay);
	m_ctrl = new MenuControlButton(this);
	m_ctrl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_ctrl->setCheckBoxStyle(MenuControlButton::CS_COLLAPSE);
	m_ctrl->setCheckable(false);
	m_ctrl->checkBox()->setChecked(true);
	m_lay->addWidget(m_ctrl);
	QWidget *container = new QWidget(this);
	m_lay->addWidget(container);
	m_contLayout = new QVBoxLayout(container);
	container->setLayout(m_contLayout);
	m_contLayout->setMargin(0);
	m_contLayout->setSpacing(0);

	connect(m_ctrl->checkBox(), SIGNAL(toggled(bool)), container, SLOT(setVisible(bool)));
}

CollapsableMenuControlButton::~CollapsableMenuControlButton() {}

void CollapsableMenuControlButton::add(QWidget *ch)
{
	ch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_contLayout->addWidget(ch);
}

MenuControlButton *CollapsableMenuControlButton::getControlBtn() { return m_ctrl; }

#include "moc_menucontrolbutton.cpp"
