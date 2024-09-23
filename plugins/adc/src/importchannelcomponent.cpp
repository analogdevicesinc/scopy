#include "importchannelcomponent.h"
#include "timeplotcomponentchannel.h"
#include "menusectionwidget.h"

#include <QLineEdit>

using namespace scopy;
using namespace scopy::adc;

ImportChannelComponent::ImportChannelComponent(ImportFloatChannelNode *node, QPen pen, QWidget *parent)
	: ChannelComponent(node->recipe().name, pen, parent)
{

	m_plotChannelCmpt = new TimePlotComponentChannel(this, node->recipe().targetPlot, this);
	m_timePlotChannelComponent = dynamic_cast<TimePlotComponentChannel *>(m_plotChannelCmpt);
	connect(m_chData, &ChannelData::newData, m_timePlotChannelComponent, &TimePlotComponentChannel::onNewData);

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

ImportChannelComponent::~ImportChannelComponent() {}

void ImportChannelComponent::onInit()
{
	m_yCtrl->setMin(-1024);
	m_yCtrl->setMax(1024);

	auto rec = m_node->recipe();
	addChannelToPlot();

	chData()->onNewData(rec.x.data(), rec.y.data(), rec.x.size(), true);
	m_timePlotChannelComponent->refreshData(true);
}

QWidget *ImportChannelComponent::createMenu(QWidget *parent)
{
	initMenu(parent);
	QWidget *yaxismenu = createYAxisMenu(m_menu);
	QWidget *curvemenu = createCurveMenu(m_menu);
	// QWidget *measuremenu = m_measureMgr->createMeasurementMenu(w);
	m_menu->header()->title()->setEnabled(true);
	connect(m_menu->header()->title(), &QLineEdit::textChanged, this, [=](QString s) { m_ctrl->setName(s); });

	QPushButton *m_forget = new QPushButton("Remove reference channel");
	StyleHelper::BasicButton(m_forget);
	connect(m_forget, &QAbstractButton::clicked, this, &ImportChannelComponent::forgetChannel);

	m_menu->add(yaxismenu, "yaxis");
	m_menu->add(curvemenu, "curve");
	m_menu->add(m_forget, "forget", gui::MenuWidget::MA_BOTTOMLAST);

	return m_menu;
}

QWidget *ImportChannelComponent::createYAxisMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section =
		new MenuSectionCollapseWidget("Y-AXIS", MenuCollapseSection::MHCW_ONOFF, parent);

	m_yCtrl = new MenuPlotAxisRangeControl(m_timePlotChannelComponent->m_timePlotYAxis, section);
	m_autoscaleBtn = new QPushButton(tr("AUTOSCALE"), section);
	StyleHelper::BasicButton(m_autoscaleBtn);
	m_autoscaler = new PlotAutoscaler(this);
	m_autoscaler->addChannels(m_timePlotChannelComponent->m_timePlotCh);

	connect(m_autoscaler, &PlotAutoscaler::newMin, m_yCtrl, &MenuPlotAxisRangeControl::setMin);
	connect(m_autoscaler, &PlotAutoscaler::newMax, m_yCtrl, &MenuPlotAxisRangeControl::setMax);

	connect(m_yCtrl, &MenuPlotAxisRangeControl::intervalChanged, this, [=](double min, double max) {
		m_timePlotChannelComponent->m_xyPlotYAxis->setInterval(m_yCtrl->min(), m_yCtrl->max());
	});

	connect(section->collapseSection()->header(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yLock = b;
		m_timePlotChannelComponent->lockYAxis(!b);
	});

	connect(m_autoscaleBtn, &QAbstractButton::pressed, m_autoscaler, &PlotAutoscaler::autoscale);

	section->contentLayout()->addWidget(m_yCtrl);
	section->contentLayout()->addWidget(m_autoscaleBtn);

	return section;
}

QWidget *ImportChannelComponent::createCurveMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section =
		new MenuSectionCollapseWidget("CURVE", MenuCollapseSection::MHCW_NONE, parent);
	section->contentLayout()->setSpacing(10);

	m_curvemenu = new MenuPlotChannelCurveStyleControl(section);
	section->contentLayout()->addWidget(m_curvemenu);
	return section;
}

void ImportChannelComponent::addChannelToPlot()
{
	m_yCtrl->addAxis(m_timePlotChannelComponent->m_timePlotYAxis);
	m_autoscaler->addChannels(m_timePlotChannelComponent->m_timePlotCh);
}

void ImportChannelComponent::removeChannelFromPlot()
{
	m_yCtrl->removeAxis(m_timePlotChannelComponent->m_timePlotYAxis);
	m_autoscaler->removeChannels(m_timePlotChannelComponent->m_timePlotCh);
}

void ImportChannelComponent::forgetChannel()
{
	AcqTreeNode *treeRoot = m_node->treeRoot();
	treeRoot->removeTreeChild(m_node);
}
