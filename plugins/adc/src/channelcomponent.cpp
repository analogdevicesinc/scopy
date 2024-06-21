#include "channelcomponent.h"
#include "qlineedit.h"
#include <gui/plotchannel.h>
#include <gui/plotaxis.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <QLoggingCategory>
#include <timeplotcomponentchannel.h>
#include <gui/widgets/menuheader.h>
#include <gui/widgets/menuwidget.h>

Q_LOGGING_CATEGORY(CAT_TIME_CHANNELCOMPONENT, "TimeChannelComponent");

using namespace scopy;
using namespace gui;
using namespace scopy::adc;
ChannelComponent::ChannelComponent(QString ch, TimePlotComponent *m_plot, QPen pen, QWidget *parent)
	: QWidget(parent)
	, ToolComponent()
	, m_channelName(ch)
	, m_pen(pen)
	, m_chData(new ChannelData(this))
	, m_plotChannelCmpt(new TimePlotComponentChannel(this, m_plot, this))
	, m_menu(nullptr)
{

	m_ctrl = nullptr;
	connect(m_chData, &ChannelData::newData, m_plotChannelCmpt, &TimePlotComponentChannel::onNewData);
	m_name = m_channelName;
	m_enabled = true;
}

ChannelComponent::~ChannelComponent() {
}

void ChannelComponent::onStart() {}

void ChannelComponent::onStop() {}

void ChannelComponent::onInit() {}

void ChannelComponent::onDeinit() {
	m_plotChannelCmpt->deinitPlotComponent();
}

QPen ChannelComponent::pen() const { return m_pen; }

ChannelData *ChannelComponent::chData() const { return m_chData; }

TimePlotComponentChannel *ChannelComponent::plotChannelCmpt() const { return m_plotChannelCmpt; }

void ChannelComponent::setPlotChannelCmpt(TimePlotComponentChannel *newPlotChannelCmpt)
{
	m_plotChannelCmpt = newPlotChannelCmpt;
}

MenuControlButton *ChannelComponent::ctrl()
{
	return m_ctrl;
}

void ChannelComponent::addChannelToPlot()
{

}

void ChannelComponent::removeChannelFromPlot()
{

}

MenuWidget *ChannelComponent::menu()
{
	return m_menu;
}

void ChannelComponent::enable()
{
	m_plotChannelCmpt->enable();
	ToolComponent::enable();
}

void ChannelComponent::disable()
{
	m_plotChannelCmpt->disable();
	ToolComponent::disable();
}

void ChannelComponent::initMenu(QWidget *parent){
	m_menu = new MenuWidget(m_channelName, m_pen, parent);
}

void ChannelComponent::createMenuControlButton(ChannelComponent *c, QWidget *parent)
{
	c->m_ctrl = new MenuControlButton(parent);
	c->m_ctrl->setName(c->m_channelName);
	c->m_ctrl->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	c->m_ctrl->setOpenMenuChecksThis(true);
	c->m_ctrl->setDoubleClickToOpenMenu(true);
	c->m_ctrl->setColor(c->pen().color());
	c->m_ctrl->button()->setVisible(false);
	c->m_ctrl->setCheckable(true);

	connect(c->m_ctrl->checkBox(), &QCheckBox::toggled, c, [=](bool b) {
		if(b) {
			c->enable();
		} else {
			c->disable();
		}
	c->plotChannelCmpt()->m_plotComponent->replot();
	});
	c->m_ctrl->checkBox()->setChecked(true);

}

ImportChannelComponent::ImportChannelComponent(ImportFloatChannelNode *node, QPen pen, QWidget *parent) : ChannelComponent(node->recipe().name, node->recipe().targetPlot, pen, parent)
{

	m_node = node;
	m_channelName = node->name();
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	widget = createMenu(this);
	m_lay->addWidget(widget);
	setLayout(m_lay);
	createMenuControlButton(this);

}

ImportChannelComponent::~ImportChannelComponent()
{


}

void ImportChannelComponent::onInit()
{
	m_yCtrl->setMin(-1024);
	m_yCtrl->setMax(1024);

	auto rec = m_node->recipe();
	addChannelToPlot();

	chData()->onNewData(rec.x.data(), rec.y.data(), rec.x.size(), true);
	m_plotChannelCmpt->refreshData(true);
}

QWidget *ImportChannelComponent::createMenu(QWidget *parent)
{
	initMenu(parent);
	QWidget *yaxismenu = createYAxisMenu(m_menu);
	QWidget *curvemenu = createCurveMenu(m_menu);
	// QWidget *measuremenu = m_measureMgr->createMeasurementMenu(w);
	m_menu->header()->title()->setEnabled(true);
	connect(m_menu->header()->title(), &QLineEdit::textChanged, this, [=](QString s) {
		m_ctrl->setName(s);
	});

	QPushButton *m_forget = new QPushButton("Remove reference channel");
	StyleHelper::BlueButton(m_forget);
	connect(m_forget, &QAbstractButton::clicked, this, &ImportChannelComponent::forgetChannel);

	m_menu->add(yaxismenu,"yaxis");
	m_menu->add(curvemenu,"curve");
	m_menu->add(m_forget,"forget",gui::MenuWidget::MA_BOTTOMLAST);

	return m_menu;
}

QWidget *ImportChannelComponent::createYAxisMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("Y-AXIS", MenuCollapseSection::MHCW_NONE, parent);

	m_yCtrl = new MenuPlotAxisRangeControl(m_plotChannelCmpt->m_timePlotYAxis, section);
	m_autoscaleBtn = new QPushButton(tr("AUTOSCALE"), section);
	StyleHelper::BlueButton(m_autoscaleBtn);
	m_autoscaler = new PlotAutoscaler(this);
	m_autoscaler->addChannels(m_plotChannelCmpt->m_timePlotCh);

	connect(m_autoscaler, &PlotAutoscaler::newMin, m_yCtrl, &MenuPlotAxisRangeControl::setMin);
	connect(m_autoscaler, &PlotAutoscaler::newMax, m_yCtrl, &MenuPlotAxisRangeControl::setMax);

	connect(m_yCtrl, &MenuPlotAxisRangeControl::intervalChanged, this, [=](double min, double max) {
		m_plotChannelCmpt->m_xyPlotYAxis->setInterval(m_yCtrl->min(), m_yCtrl->max());
	});

	connect(section->collapseSection()->header(), &QAbstractButton::toggled, this, [=](bool b){
		m_yLock = b;
		m_plotChannelCmpt->lockYAxis(!b);
	});


	connect(m_autoscaleBtn, &QAbstractButton::pressed, m_autoscaler, &PlotAutoscaler::autoscale);

	section->contentLayout()->addWidget(m_yCtrl);
	section->contentLayout()->addWidget(m_autoscaleBtn);

	return section;
}

QWidget *ImportChannelComponent::createCurveMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("CURVE", MenuCollapseSection::MHCW_NONE, parent);
	section->contentLayout()->setSpacing(10);

	m_curvemenu = new MenuPlotChannelCurveStyleControl(section);
	section->contentLayout()->addWidget(m_curvemenu);
	return section;
}

void ImportChannelComponent::forgetChannel() {
	AcqTreeNode* treeRoot= m_node->treeRoot();
	treeRoot->removeTreeChild(m_node);
}
