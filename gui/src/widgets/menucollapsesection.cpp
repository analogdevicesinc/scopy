#include <widgets/menucollapsesection.h>
#include <smallOnOffSwitch.h>

using namespace scopy;

MenuCollapseHeader::MenuCollapseHeader(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style, QWidget *parent)
    : QAbstractButton(parent)
{
	lay = new QHBoxLayout(this);
	lay->setMargin(0);
	lay->setContentsMargins(0,2,0,6);
	lay->setSpacing(0);

	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
	setCheckable(true);
	setLayout(lay);

	m_label = new QLabel(title, this);
	StyleHelper::MenuCollapseHeaderLabel(m_label, "menuCollapseLabel");


	switch(style){
	case MenuCollapseSection::MHCW_ARROW:
		m_ctrl = new QCheckBox(this);
		StyleHelper::CollapseCheckbox(dynamic_cast<QCheckBox*>(m_ctrl), "menuCollapseButton");
		connect(this, &QAbstractButton::toggled, this, [=](bool b) {m_ctrl->setChecked(!b);});
		m_ctrl->setChecked(false);
		break;
	case MenuCollapseSection::MHCW_ONOFF:
		m_ctrl = new SmallOnOffSwitch(this);
		StyleHelper::MenuOnOffSwitchButton(dynamic_cast<SmallOnOffSwitch*>(m_ctrl), "menuCollapseButton");
		connect(this, &QAbstractButton::toggled, m_ctrl, &QAbstractButton::toggled);
		m_ctrl->setChecked(true);
		break;
	default:
		m_ctrl = new QCheckBox(this);
		m_ctrl->setVisible(false);
		break;
	}

	setChecked(true);
	m_ctrl->setAttribute(Qt::WA_TransparentForMouseEvents);

	lay->addWidget(m_label);
	lay->addSpacerItem(new QSpacerItem(40,40,QSizePolicy::Expanding,QSizePolicy::Maximum));
	lay->addWidget(m_ctrl);

}

MenuCollapseHeader::~MenuCollapseHeader() {

}

MenuCollapseSection::MenuCollapseSection(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style, QWidget *parent)
    : QWidget(parent) {

	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	setLayout(m_lay);
	m_header = new MenuCollapseHeader(title, style, this);
	m_lay->addWidget(m_header);
	QWidget *container = new QWidget(this);
	m_lay->addWidget(container);
	m_contLayout = new QVBoxLayout(container);
	container->setLayout(m_contLayout);
	m_contLayout->setMargin(0);
	m_contLayout->setSpacing(0);

	StyleHelper::MenuCollapseSection(this,"menuCollapse");
	connect(m_header, &QAbstractButton::toggled, container, &QWidget::setVisible);
}

MenuCollapseSection::~MenuCollapseSection() {}
QAbstractButton *MenuCollapseSection::header() { return m_header; }

QVBoxLayout* MenuCollapseSection::contentLayout() const { return m_contLayout; }


//void MenuCollapseSection::add(QWidget *ch) {
//	m_contLayout->addWidget(ch);
//}

