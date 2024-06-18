#include "grtimechannelcomponent.h"
#include <pluginbase/preferences.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuheader.h>

#include <gr-util/griiofloatchannelsrc.h>
#include <gr-util/grsignalpath.h>

#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetfactory.h>
#include <timeplotcomponentchannel.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>

Q_LOGGING_CATEGORY(CAT_GRTIMECHANNELCOMPONENT, "GRTimeChannelComponent");

using namespace scopy;
using namespace scopy::grutil;
using namespace scopy::adc;

GRTimeChannelComponent::GRTimeChannelComponent(GRIIOFloatChannelNode *node, TimePlotComponent *m_plot,
					       GRTimeSinkComponent *grtsc, QPen pen, QWidget *parent)
	: ChannelComponent(node->name(), m_plot, pen, parent)

{
	m_node = node;
	m_src = node->src();

	m_grtch = new GRTimeChannelSigpath(grtsc->name(), this, node, this);

	m_running = false;
	m_autoscaleEnabled = false;

	m_scaleAvailable = m_src->scaleAttributeAvailable(); // query from GRIIOFloatChannel;
	m_unit = "Volts";	 // query from GRIIOFloatChannel;

	m_channelName = node->name();

	m_measureMgr = new TimeMeasureManager(this);
	m_measureMgr->initMeasure(m_pen);
	m_measureMgr->getModel()->setAdcBitCount(m_src->getFmt()->bits);

	widget = createMenu();

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	widget = createMenu(this);
	m_lay->addWidget(widget);
	setLayout(m_lay);

	createMenuControlButton(this);
}

GRTimeChannelComponent::~GRTimeChannelComponent() {}

QWidget *GRTimeChannelComponent::createYAxisMenu(QWidget *parent)
{
	MenuSectionWidget *yaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yaxis = new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, yaxiscontainer);

	// Y-MODE
	m_ymodeCb = new MenuCombo("YMODE", yaxis);
	auto cb = m_ymodeCb->combo();
	cb->addItem("ADC Counts", YMODE_COUNT);
	cb->addItem("% Full Scale", YMODE_FS);

	m_scaleWidget = nullptr;
	if(m_scaleAvailable) {
		cb->addItem(m_unit, YMODE_SCALE);
		m_scaleWidget = IIOWidgetFactory::buildAttrForChannel(m_src->channel(), m_src->scaleAttribute(),this);
	}

	m_yAxisCtrl = new MenuOnOffSwitch("LOCK Y-Axis");
	m_yCtrl = new MenuPlotAxisRangeControl(m_plotChannelCmpt->m_timePlotYAxis, yaxis);
	m_autoscaleBtn = new MenuOnOffSwitch(tr("AUTOSCALE"), yaxis, false);
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

	connect(m_autoscaleBtn->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yCtrl->setEnabled(!b);
		m_autoscaleEnabled = b;
		toggleAutoScale();
	});

	yaxis->contentLayout()->addWidget(m_yAxisCtrl);
	yaxis->contentLayout()->addWidget(m_autoscaleBtn);
	yaxis->contentLayout()->addWidget(m_yCtrl);
	yaxis->contentLayout()->addWidget(m_ymodeCb);
	if(m_scaleWidget)
		yaxis->contentLayout()->addWidget(m_scaleWidget);

	yaxiscontainer->contentLayout()->addWidget(yaxis);

	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		auto mode = cb->itemData(idx).toInt();
		setYMode(static_cast<YMode>(mode));
	});

	connect(this, &GRTimeChannelComponent::yModeChanged, this, [=]() {
		int idx = cb->currentIndex();
		int itemcount = cb->count();
		for(int i = 0; i < itemcount; i++) {
			if(cb->itemData(i) == m_ymode) {
				idx = i;
				break;
			}
		}
		cb->setCurrentIndex(idx);
	});

	return yaxiscontainer;
}

QWidget *GRTimeChannelComponent::createCurveMenu(QWidget *parent)
{
	MenuSectionWidget *curvecontainer = new MenuSectionWidget(parent);
	m_curveSection = new MenuCollapseSection("CURVE", MenuCollapseSection::MHCW_NONE, curvecontainer);
	m_curveSection->contentLayout()->setSpacing(10);

	m_curvemenu = new MenuPlotChannelCurveStyleControl(m_curveSection);
	curvecontainer->contentLayout()->addWidget(m_curveSection);
	m_curveSection->contentLayout()->addWidget(m_curvemenu);
	return curvecontainer;
}


QPushButton *GRTimeChannelComponent::createSnapshotButton(QWidget *parent)
{
	QPushButton *snapBtn = new QPushButton("Snapshot", parent);
	StyleHelper::BlueButton(snapBtn);

	connect(snapBtn, &QPushButton::clicked, this, [=]() {
		std::vector<float> x, y;
		auto data = m_plotChannelCmpt->m_timePlotCh->curve()->data();
		for(int i = 0; i < data->size(); i++) {
			x.push_back(data->sample(i).x());
			y.push_back(data->sample(i).y());
		}
		SnapshotRecipe rec{x, y, m_plotChannelCmpt->m_plotComponent, "REF - " + m_channelName};
		AcqTreeNode* treeRoot= m_node->treeRoot();
		ImportFloatChannelNode *snap = new ImportFloatChannelNode(rec,treeRoot);
		treeRoot->addTreeChild(snap);

	});

	snapBtn->setEnabled(false);
	return snapBtn;
}


QWidget *GRTimeChannelComponent::createMenu(QWidget *parent)
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
	QWidget *attrmenu = createAttrMenu(w);
	QWidget *measuremenu = m_measureMgr->createMeasurementMenu(w);
	m_snapBtn = createSnapshotButton(w);

	lay->addWidget(header);
	lay->addWidget(scroll);
	m_layScroll->addWidget(yaxismenu);
	m_layScroll->addWidget(curvemenu);
	m_layScroll->addWidget(attrmenu);
	m_layScroll->addWidget(measuremenu);
	m_layScroll->addWidget(m_snapBtn);

	m_layScroll->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	return w;
}

QWidget *GRTimeChannelComponent::createAttrMenu(QWidget *parent)
{
	MenuSectionWidget *attrcontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *attr =
		new MenuCollapseSection("ATTRIBUTES", MenuCollapseSection::MHCW_NONE, attrcontainer);
	QList<IIOWidget *> attrWidgets = IIOWidgetFactory::buildAllAttrsForChannel(m_src->channel());

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 10); // bottom margin

	for(auto w : attrWidgets) {
		layout->addWidget(w);
	}

	attr->contentLayout()->addLayout(layout);
	attrcontainer->contentLayout()->addWidget(attr);
	attr->header()->setChecked(false);
	return attrcontainer;
}

void GRTimeChannelComponent::onStart()
{
	m_running = true;
	// m_measureMgr->getModel()->setSampleRate(m_plotSampleRate);
	toggleAutoScale();
}

void GRTimeChannelComponent::onStop()
{
	m_running = false;
	toggleAutoScale();
	if(m_autoscaleEnabled) {
		m_autoscaler->autoscale();
	}
}

void GRTimeChannelComponent::toggleAutoScale()
{
	if(m_running && m_autoscaleEnabled) {
		m_autoscaler->start();
	} else {
		m_autoscaler->stop();
	}
}

void GRTimeChannelComponent::setYModeHelper(YMode mode)
{
	double scale = 1;
	double offset = 0;
	double ymin = -1;
	double ymax = 1;
	const iio_data_format *fmt = m_src->getFmt(); // get from iio-node interface

	switch(mode) {
	case YMODE_COUNT:
		if(m_scaleAvailable) {
			m_scaleWidget->setVisible(false);
		}
		scale = 1;
		if(fmt->is_signed) {
			ymin = -(float)((int64_t)1 << (fmt->bits - 1));
			ymax = (float)((int64_t)1 << (fmt->bits - 1));
		} else {
			ymin = 0;
			ymax = 1 << (fmt->bits);
		}
		//		m_plotCh->yAxis()->setUnits("Counts");
		break;
	case YMODE_FS:
		if(m_scaleAvailable) {
			m_scaleWidget->setVisible(false);
		}
		scale = 1.0 / ((float)((uint64_t)1 << fmt->bits));
		if(fmt->is_signed) {
			ymin = -0.5;
			ymax = 0.5;
		} else {
			ymin = 0;
			ymax = 1;
		}
		//m_plotCh->yAxis()->setUnits("");
		break;
	case YMODE_SCALE:
		if(m_scaleAvailable) {
			scale = m_scaleWidget->getDataStrategy()->data().toDouble();
			m_scaleWidget->setVisible(true);
		}
		if(fmt->is_signed) {
			ymin = -(float)((int64_t)1 << (fmt->bits - 1));
			ymax = (float)((int64_t)1 << (fmt->bits - 1));
		} else {
			ymin = 0;
			ymax = (1 << (fmt->bits));
		}

		ymin = ymin * scale;
		ymax = ymax * scale;

		//m_plotCh->yAxis()->setUnits(m_unit);

		break;
	default:
		break;
	}

	m_yCtrl->setMin(ymin);
	m_yCtrl->setMax(ymax);
	m_grtch->m_scOff->setScale(scale);
	m_grtch->m_scOff->setOffset(offset);
}

void GRTimeChannelComponent::addChannelToPlot()
{
	m_curvemenu->addChannels(m_plotChannelCmpt->m_timePlotCh);
	m_curvemenu->addChannels(m_plotChannelCmpt->m_xyPlotCh);
}

void GRTimeChannelComponent::removeChannelFromPlot()
{
	m_curvemenu->removeChannels(m_plotChannelCmpt->m_timePlotCh);
	m_curvemenu->removeChannels(m_plotChannelCmpt->m_xyPlotCh);
}

bool GRTimeChannelComponent::scaleAvailable() const
{
	return m_scaleAvailable;
}

bool GRTimeChannelComponent::yLock() const {
	return m_yLock;
}

double GRTimeChannelComponent::yMin() const {
	return m_yCtrl->min();
}

double GRTimeChannelComponent::yMax() const {
	return m_yCtrl->max();
}


MeasureManagerInterface *GRTimeChannelComponent::getMeasureManager() { return m_measureMgr; }

GRSignalPath *GRTimeChannelComponent::sigpath() { return m_grtch->m_signalPath; }

void GRTimeChannelComponent::insertMenuWidget(QWidget *w)
{
	m_curveSection->contentLayout()->addWidget(w);
}

QVBoxLayout *GRTimeChannelComponent::menuLayout()
{
	return m_layScroll;
}

void GRTimeChannelComponent::enable()
{
	m_grtch->m_signalPath->setEnabled(true);
	m_plotChannelCmpt->enable();
	ChannelComponent::enable();
}

void GRTimeChannelComponent::disable()
{
	m_grtch->m_signalPath->setEnabled(false);
	m_plotChannelCmpt->disable();
	ChannelComponent::disable();
}

void GRTimeChannelComponent::onInit()
{
	// Defaults
	addChannelToPlot();

	m_yCtrl->setMin(-1.0);
	m_yCtrl->setMax(1.0);
	auto v = Preferences::get("adc_default_y_mode").toInt();
	m_ymodeCb->combo()->setCurrentIndex(v);
	setYMode(static_cast<YMode>(v));
}

void GRTimeChannelComponent::onDeinit() {}

void GRTimeChannelComponent::onNewData(const float *xData, const float *yData, size_t size, bool copy)
{
	m_grtch->onNewData(xData, yData, size, copy);
	auto model = m_measureMgr->getModel();
	model->setDataSource(yData, size);
	model->measure();
	m_snapBtn->setEnabled(true);
}


bool GRTimeChannelComponent::sampleRateAvailable()
{
	return m_src->samplerateAttributeAvailable();
}

double GRTimeChannelComponent::sampleRate()
{
	return m_src->readSampleRate();
}

YMode GRTimeChannelComponent::ymode() const
{
	return m_ymode;
}

void GRTimeChannelComponent::setYMode(YMode newYmode)
{
	if (m_ymode == newYmode)
		return;
	m_ymode = newYmode;
	setYModeHelper(newYmode);
	Q_EMIT yModeChanged();
}
