#include "grtimechannelcomponent.h"
#include <pluginbase/preferences.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuheader.h>

#include <gr-util/griiofloatchannelsrc.h>
#include <gr-util/grsignalpath.h>

#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <timeplotcomponentchannel.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>

Q_LOGGING_CATEGORY(CAT_GRTIMECHANNELCOMPONENT, "GRTimeChannelComponent");

using namespace scopy;
using namespace scopy::grutil;
using namespace scopy::adc;

GRTimeChannelComponent::GRTimeChannelComponent(GRIIOFloatChannelNode *node, TimePlotComponent *m_plot,
					       GRTimeSinkComponent *grtsc, QPen pen, QWidget *parent)
	: ChannelComponent(node->name(), pen, parent)

{
	m_plotChannelCmpt = new TimePlotComponentChannel(this, m_plot, this);
	m_timePlotComponentChannel = dynamic_cast<TimePlotComponentChannel *>(m_plotChannelCmpt);
	connect(m_chData, &ChannelData::newData, m_timePlotComponentChannel, &TimePlotComponentChannel::onNewData);

	m_node = node;
	m_src = node->src();

	m_grtch = new GRTimeChannelSigpath(grtsc->name(), this, node, this);
	m_running = false;
	m_autoscaleEnabled = false;

	m_scaleAvailable = m_src->scaleAttributeAvailable(); // query from GRIIOFloatChannel;
	m_unit = m_src->unit();				     // query from GRIIOFloatChannel;

	m_channelName = node->name();

	m_measureMgr = new TimeMeasureManager(this);
	m_measureMgr->initMeasure(m_pen);
	m_measureMgr->getModel()->setAdcBitCount(m_src->getFmt()->bits);

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
	m_yaxisMenu = new MenuSectionCollapseWidget("Y-AXIS", MenuCollapseSection::MHCW_ONOFF, parent);

	// Y-MODE
	m_ymodeCb = new MenuCombo("YMODE", m_yaxisMenu);
	auto cb = m_ymodeCb->combo();
	cb->addItem("ADC Counts", YMODE_COUNT);
	cb->addItem("% Full Scale", YMODE_FS);

	m_scaleWidget = nullptr;
	if(m_scaleAvailable) {
		cb->addItem(m_unit.name, YMODE_SCALE);
		m_scaleWidget = IIOWidgetBuilder(m_yaxisMenu)
					.channel(m_src->channel())
					.attribute(m_src->scaleAttribute())
					.buildSingle();
	}

	m_yCtrl = new MenuPlotAxisRangeControl(m_timePlotComponentChannel->m_timePlotYAxis, m_yaxisMenu);
	m_autoscaleBtn = new MenuOnOffSwitch(tr("AUTOSCALE"), m_yaxisMenu, false);
	m_autoscaler = new PlotAutoscaler(this);
	m_autoscaler->addChannels(m_timePlotComponentChannel->m_timePlotCh);

	connect(m_autoscaler, &PlotAutoscaler::newMin, m_yCtrl, &MenuPlotAxisRangeControl::setMin);
	connect(m_autoscaler, &PlotAutoscaler::newMax, m_yCtrl, &MenuPlotAxisRangeControl::setMax);

	connect(m_yCtrl, &MenuPlotAxisRangeControl::intervalChanged, this, [=](double min, double max) {
		m_timePlotComponentChannel->m_xyPlotYAxis->setInterval(m_yCtrl->min(), m_yCtrl->max());
	});

	connect(m_yaxisMenu->collapseSection()->header(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yLock = b;
		m_timePlotComponentChannel->lockYAxis(!b);
	});

	connect(m_autoscaleBtn->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yCtrl->setEnabled(!b);
		m_autoscaleEnabled = b;
		toggleAutoScale();
	});

	m_yaxisMenu->contentLayout()->addWidget(m_autoscaleBtn);
	m_yaxisMenu->contentLayout()->addWidget(m_yCtrl);
	m_yaxisMenu->contentLayout()->addWidget(m_ymodeCb);
	if(m_scaleWidget)
		m_yaxisMenu->contentLayout()->addWidget(m_scaleWidget);

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

	return m_yaxisMenu;
}

QWidget *GRTimeChannelComponent::createCurveMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section =
		new MenuSectionCollapseWidget("CURVE", MenuCollapseSection::MHCW_NONE, parent);
	section->contentLayout()->setSpacing(10);

	m_curvemenu = new MenuPlotChannelCurveStyleControl(section);
	section->contentLayout()->addWidget(m_curvemenu);
	return section;
}

QPushButton *GRTimeChannelComponent::createSnapshotButton(QWidget *parent)
{
	QPushButton *snapBtn = new QPushButton("Snapshot", parent);
	StyleHelper::BasicButton(snapBtn);

	connect(snapBtn, &QPushButton::clicked, this, [=]() {
		std::vector<float> x, y;
		auto data = m_timePlotComponentChannel->m_timePlotCh->curve()->data();
		for(int i = 0; i < data->size(); i++) {
			x.push_back(data->sample(i).x());
			y.push_back(data->sample(i).y());
		}
		SnapshotRecipe rec{x, y, m_timePlotComponentChannel->m_plotComponent, "REF - " + m_channelName};
		AcqTreeNode *treeRoot = m_node->treeRoot();
		ImportFloatChannelNode *snap = new ImportFloatChannelNode(rec, treeRoot);
		treeRoot->addTreeChild(snap);
	});

	snapBtn->setEnabled(false);
	return snapBtn;
}

QWidget *GRTimeChannelComponent::createMenu(QWidget *parent)
{
	ChannelComponent::initMenu(parent);
	QWidget *yaxismenu = createYAxisMenu(m_menu);
	QWidget *curvemenu = createCurveMenu(m_menu);
	QWidget *attrmenu = createAttrMenu(m_menu);
	QWidget *measuremenu = m_measureMgr->createMeasurementMenu(m_menu);
	m_snapBtn = createSnapshotButton(m_menu);

	m_menu->add(yaxismenu, "yaxis");
	m_menu->add(curvemenu, "curve");
	m_menu->add(attrmenu, "attr");
	m_menu->add(measuremenu, "measure");
	m_menu->add(m_snapBtn, "snap", MenuWidget::MA_BOTTOMLAST);

	return m_menu;
}

QWidget *GRTimeChannelComponent::createAttrMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section =
		new MenuSectionCollapseWidget("ATTRIBUTES", MenuCollapseSection::MHCW_NONE, parent);
	QList<IIOWidget *> attrWidgets = IIOWidgetBuilder(section).channel(m_src->channel()).buildAll();

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 10); // bottom margin

	for(auto w : attrWidgets) {
		layout->addWidget(w);
	}

	section->contentLayout()->addLayout(layout);
	section->setCollapsed(true);
	return section;
}

void GRTimeChannelComponent::onStart()
{
	m_running = true;
	m_grtch->m_signalPath->setEnabled(true);
	toggleAutoScale();
}

void GRTimeChannelComponent::onStop()
{
	m_running = false;
	m_grtch->m_signalPath->setEnabled(false);
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
		m_timePlotComponentChannel->m_timePlotYAxis->setUnits("");
		m_timePlotComponentChannel->m_timePlotYAxis->setUnits("");
		m_timePlotComponentChannel->m_timePlotYAxis->scaleDraw()->setFloatPrecision(3);
		m_timePlotComponentChannel->m_timePlotYAxis->getFormatter()->setTwoDecimalMode(false);
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
		m_timePlotComponentChannel->m_timePlotYAxis->setUnits("");
		m_timePlotComponentChannel->m_timePlotYAxis->scaleDraw()->setFloatPrecision(3);
		m_timePlotComponentChannel->m_timePlotYAxis->getFormatter()->setTwoDecimalMode(false);
		break;
	case YMODE_SCALE:
		if(m_scaleAvailable) {
			scale = m_scaleWidget->read().first.toDouble();
			m_scaleWidget->setVisible(true);
		}
		if(fmt->is_signed) {
			ymin = -(float)((int64_t)1 << (fmt->bits - 1));
			ymax = (float)((int64_t)1 << (fmt->bits - 1));
		} else {
			ymin = 0;
			ymax = (1 << (fmt->bits));
		}

		scale = scale / 1000.0; // target value is in mV

		ymin = ymin * scale;
		ymax = ymax * scale;

		m_timePlotComponentChannel->m_timePlotYAxis->setUnits(unit().symbol);
		m_timePlotComponentChannel->m_timePlotYAxis->scaleDraw()->setFloatPrecision(3);
		m_timePlotComponentChannel->m_timePlotYAxis->getFormatter()->setTwoDecimalMode(true);

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
	m_yCtrl->addAxis(m_timePlotComponentChannel->m_timePlotYAxis);
	m_curvemenu->addChannels(m_timePlotComponentChannel->m_timePlotCh);
	m_curvemenu->addChannels(m_timePlotComponentChannel->m_xyPlotCh);
	m_autoscaler->addChannels(m_timePlotComponentChannel->m_timePlotCh);
}

void GRTimeChannelComponent::removeChannelFromPlot()
{
	m_yCtrl->removeAxis(m_timePlotComponentChannel->m_timePlotYAxis);
	m_curvemenu->removeChannels(m_timePlotComponentChannel->m_timePlotCh);
	m_curvemenu->removeChannels(m_timePlotComponentChannel->m_xyPlotCh);
	m_autoscaler->removeChannels(m_timePlotComponentChannel->m_timePlotCh);
}

IIOUnit GRTimeChannelComponent::unit() const { return m_unit; }

void GRTimeChannelComponent::enable()
{
	ChannelComponent::enable();
	Q_EMIT sigpath()->requestRebuild();
}

void GRTimeChannelComponent::disable()
{
	ChannelComponent::disable();
	Q_EMIT sigpath()->requestRebuild();
}

bool GRTimeChannelComponent::scaleAvailable() const { return m_scaleAvailable; }

bool GRTimeChannelComponent::yLock() const { return m_yLock; }

double GRTimeChannelComponent::yMin() const { return m_yCtrl->min(); }

double GRTimeChannelComponent::yMax() const { return m_yCtrl->max(); }

MeasureManagerInterface *GRTimeChannelComponent::getMeasureManager() { return m_measureMgr; }

GRSignalPath *GRTimeChannelComponent::sigpath() { return m_grtch->sigpath(); }

QVBoxLayout *GRTimeChannelComponent::menuLayout() { return m_layScroll; }

void GRTimeChannelComponent::onInit()
{
	// Defaults
	addChannelToPlot();

	m_yaxisMenu->setCollapsed(true);
	m_yCtrl->setMin(-1.0);
	m_yCtrl->setMax(1.0);

	m_ymode = static_cast<adc::YMode>(-1);
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

bool GRTimeChannelComponent::sampleRateAvailable() { return m_src->samplerateAttributeAvailable(); }

double GRTimeChannelComponent::sampleRate() { return m_src->readSampleRate(); }

void GRTimeChannelComponent::setSamplingInfo(SamplingInfo p)
{
	ChannelComponent::setSamplingInfo(p);
	m_measureMgr->getModel()->setSampleRate(p.sampleRate);
}

YMode GRTimeChannelComponent::ymode() const { return m_ymode; }

void GRTimeChannelComponent::setYMode(YMode newYmode)
{
	if(m_ymode == newYmode)
		return;
	m_ymode = newYmode;
	setYModeHelper(newYmode);
	Q_EMIT yModeChanged();
}
