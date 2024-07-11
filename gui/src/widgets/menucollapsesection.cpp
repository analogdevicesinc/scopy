#include "qlineedit.h"
#include <smallOnOffSwitch.h>
#include <style.h>
#include <widgets/menucollapsesection.h>

using namespace scopy;

MenuCollapseHeader::MenuCollapseHeader(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style,
				       QWidget *parent)
	: QAbstractButton(parent)
{
	lay = new QHBoxLayout(this);
	lay->setMargin(0);
	lay->setContentsMargins(0, 2, 0, 6);
	lay->setSpacing(0);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	setCheckable(true);
	setLayout(lay);

	m_label = new QLineEdit(title, this);
	m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_label->setEnabled(false);
	m_label->setReadOnly(false);

	StyleHelper::MenuCollapseHeaderLineEdit(m_label, "menuCollapseLabel");
	m_label->setTextMargins(0, 0, 0, 0);

	switch(style) {
	case MenuCollapseSection::MHCW_ARROW:
		m_ctrl = new QCheckBox(this);
		StyleHelper::CollapseCheckbox(dynamic_cast<QCheckBox *>(m_ctrl), "menuCollapseButton");
		connect(this, &QAbstractButton::toggled, this, [=](bool b) { m_ctrl->setChecked(!b); });
		m_ctrl->setChecked(false);
		break;
	case MenuCollapseSection::MHCW_ONOFF:
		m_ctrl = new SmallOnOffSwitch(this);
		StyleHelper::MenuOnOffSwitchButton(dynamic_cast<SmallOnOffSwitch *>(m_ctrl), "menuCollapseButton");
		connect(this, &QAbstractButton::toggled, [=](bool b) { m_ctrl->setChecked(b); });
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
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Maximum));
	lay->addWidget(m_ctrl);
}

MenuCollapseHeader::~MenuCollapseHeader() {}

QLineEdit *MenuCollapseHeader::title() { return m_label; }

MenuCollapseSection::MenuCollapseSection(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style,
					 QWidget *parent)
	: QWidget(parent)
	, m_title(title)
{

	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	setLayout(m_lay);
	m_header = new MenuCollapseHeader(m_title, style, this);
	m_lay->addWidget(m_header);
	QWidget *container = new QWidget(this);
	m_lay->addWidget(container);
	m_contLayout = new QVBoxLayout(container);
	container->setLayout(m_contLayout);
	m_contLayout->setMargin(0);
	m_contLayout->setSpacing(0);

	StyleHelper::MenuCollapseSection(this, "menuCollapse");
	connect(m_header, &QAbstractButton::toggled, container, &QWidget::setVisible);
}

MenuCollapseSection::~MenuCollapseSection() {}
QAbstractButton *MenuCollapseSection::header() { return m_header; }

QVBoxLayout *MenuCollapseSection::contentLayout() const { return m_contLayout; }

QString MenuCollapseSection::title() { return m_title; }

void MenuCollapseSection::setTitle(QString s)
{
	m_title = s;
	m_header->title()->setText(m_title);
}

#include "moc_menucollapsesection.cpp"
