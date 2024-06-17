#include "channelcomponent.h"
#include <gui/plotchannel.h>
#include <gui/plotaxis.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <QLoggingCategory>
#include <timeplotcomponentchannel.h>
#include <gui/widgets/menuheader.h>

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

void ChannelComponent::insertMenuWidget(QWidget *)
{

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
		if(b)
			c->enable();
		else
			c->disable();
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
	m_plotChannelCmpt->refreshData(false);
}

QWidget *ImportChannelComponent::createMenu(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout();

	QScrollArea *scroll = new QScrollArea(parent);
	QWidget *wScroll = new QWidget(scroll);

	m_layScroll = new QVBoxLayout();
	m_layScroll->setMargin(0);
	m_layScroll->setSpacing(10);

	wScroll->setLayout(m_layScroll);
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	// if ScrollBarAlwaysOn - layScroll->setContentsMargins(0,0,6,0);

	scroll->setWidget(wScroll);

	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	MenuHeaderWidget *header = new MenuHeaderWidget(m_channelName, m_pen, w);
	QWidget *yaxismenu = createYAxisMenu(w);
	QWidget *curvemenu = createCurveMenu(w);
	// QWidget *measuremenu = m_measureMgr->createMeasurementMenu(w);
	lay->addWidget(header);
	lay->addWidget(scroll);

	QPushButton *m_forget = new QPushButton("Remove reference channel");
	StyleHelper::BlueButton(m_forget);
	connect(m_forget, &QAbstractButton::clicked, this, &ImportChannelComponent::forgetChannel);

	m_layScroll->addWidget(yaxismenu);
	m_layScroll->addWidget(curvemenu);


	m_layScroll->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	m_layScroll->addWidget(m_forget);
	return w;
}

QWidget *ImportChannelComponent::createYAxisMenu(QWidget *parent)
{
	MenuSectionWidget *yaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yaxis = new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, yaxiscontainer);
	m_yAxisCtrl = new MenuOnOffSwitch("LOCK Y-Axis");
	m_yCtrl = new MenuPlotAxisRangeControl(m_plotChannelCmpt->m_timePlotYAxis, yaxis);
	m_autoscaleBtn = new QPushButton(tr("AUTOSCALE"), yaxis);
	StyleHelper::BlueButton(m_autoscaleBtn);
	m_autoscaler = new PlotAutoscaler(this);
	m_autoscaler->addChannels(m_plotChannelCmpt->m_timePlotCh);

	connect(m_autoscaler, &PlotAutoscaler::newMin, m_yCtrl, &MenuPlotAxisRangeControl::setMin);
	connect(m_autoscaler, &PlotAutoscaler::newMax, m_yCtrl, &MenuPlotAxisRangeControl::setMax);

	connect(m_yCtrl, &MenuPlotAxisRangeControl::intervalChanged, this, [=](double min, double max) {
		m_plotChannelCmpt->m_xyPlotYAxis->setInterval(m_yCtrl->min(), m_yCtrl->max());
	});

	connect(m_yAxisCtrl->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b){
		m_yLock = b;
		m_yCtrl->setVisible(!b);
		m_autoscaleBtn->setVisible(!b);
		m_plotChannelCmpt->setSingleYMode(b);
	});

	m_yAxisCtrl->onOffswitch()->setChecked(true);

	connect(m_autoscaleBtn, &QAbstractButton::pressed, m_autoscaler, &PlotAutoscaler::autoscale);

	yaxis->contentLayout()->addWidget(m_yAxisCtrl);
	yaxis->contentLayout()->addWidget(m_yCtrl);
	yaxis->contentLayout()->addWidget(m_autoscaleBtn);

	yaxiscontainer->contentLayout()->addWidget(yaxis);
	return yaxiscontainer;
}

QWidget *ImportChannelComponent::createCurveMenu(QWidget *parent)
{
	MenuSectionWidget *curvecontainer = new MenuSectionWidget(parent);
	m_curveSection = new MenuCollapseSection("CURVE", MenuCollapseSection::MHCW_NONE, curvecontainer);
	m_curveSection->contentLayout()->setSpacing(10);

	m_curvemenu = new MenuPlotChannelCurveStyleControl(m_curveSection);
	curvecontainer->contentLayout()->addWidget(m_curveSection);
	m_curveSection->contentLayout()->addWidget(m_curvemenu);
	return curvecontainer;
}

void ImportChannelComponent::forgetChannel() {
	AcqTreeNode* treeRoot= m_node->treeRoot();
	treeRoot->removeTreeChild(m_node);
}

void ImportChannelComponent::insertMenuWidget(QWidget *w) {
	m_layScroll->insertWidget(3,w);
}
