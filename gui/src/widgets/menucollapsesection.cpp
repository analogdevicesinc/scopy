#include "baseheaderwidget.h"
#include <smallOnOffSwitch.h>
#include <widgets/menucollapsesection.h>
#include <QLoggingCategory>
#include <compositeheaderwidget.h>

Q_LOGGING_CATEGORY(CAT_MENU_COLLAPSE_SECTION, "MenuCollapseSection")

using namespace scopy;

MenuCollapseHeader::MenuCollapseHeader(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style,
				       MenuCollapseSection::MenuHeaderWidgetType headerType, QWidget *parent)
	: QAbstractButton(parent)
{
	lay = new QHBoxLayout(this);
	lay->setMargin(0);
	lay->setContentsMargins(0, 2, 0, 6);
	lay->setSpacing(0);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	setCheckable(true);
	setLayout(lay);

	switch(headerType) {
	case MenuCollapseSection::MHW_BASEWIDGET:
		m_headerWidget = new BaseHeaderWidget(title, this);
		break;
	case MenuCollapseSection::MHW_COMPOSITEWIDGET:
		m_headerWidget = new CompositeHeaderWidget(title, this);
		break;
	}

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

	lay->addWidget(m_headerWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Maximum));
	lay->addWidget(m_ctrl);
}

MenuCollapseHeader::~MenuCollapseHeader() {}

QString MenuCollapseHeader::title()
{
	BaseHeader *baseHeader = dynamic_cast<BaseHeader *>(m_headerWidget);
	if(!baseHeader) {
		qDebug(CAT_MENU_COLLAPSE_SECTION) << "Header widget doesn't implement the BaseHeader interface!";
		return "";
	}
	return baseHeader->title();
}

QWidget *MenuCollapseHeader::headerWidget() const { return m_headerWidget; }

MenuCollapseSection::MenuCollapseSection(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style,
					 MenuCollapseSection::MenuHeaderWidgetType headerType, QWidget *parent)
	: QWidget(parent)
	, m_title(title)
{

	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	setLayout(m_lay);
	m_header = new MenuCollapseHeader(m_title, style, headerType, this);
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
	BaseHeader *baseHeader = dynamic_cast<BaseHeader *>(m_header->headerWidget());
	if(baseHeader) {
		baseHeader->setTitle(s);
	}
}

#include "moc_menucollapsesection.cpp"
