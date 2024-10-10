#include "grtimechanneladdon.h"

#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include "grdeviceaddon.h"
#include "plottracker.hpp"

#include <QComboBox>
#include <QDebug>
#include <QLoggingCategory>
#include <QVBoxLayout>

#include <gui/stylehelper.h>
#include <gui/widgets/menubigswitch.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuheader.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menusectionwidget.h>
#include <pluginbase/preferences.h>

Q_LOGGING_CATEGORY(CAT_GR_TIME_CHANNEL, "GRTimeChannel");
using namespace scopy::grutil;

GRTimeChannelAddon::GRTimeChannelAddon(QString ch, GRDeviceAddon *dev, GRTimePlotAddon *plotAddon, QPen pen,
				       QObject *parent)
	: TimeChannelAddon(ch, plotAddon, pen, parent)
	, m_dev(dev)
{

	int yPlotAxisPosition = Preferences::get("adc_plot_yaxis_label_position").toInt();
	int yPlotAxisHandle = Preferences::get("adc_plot_yaxis_handle_position").toInt();

	m_running = false;
	m_enabled = false;
	m_autoscaleEnabled = false;
	m_signalPath = new GRSignalPath("time_" + dev->getName() + ch, this);
	m_grch = new GRIIOFloatChannelSrc(dev->src(), ch, m_signalPath);
	m_signalPath->append(m_grch);
	m_scOff = new GRScaleOffsetProc(m_signalPath);
	m_signalPath->append(m_scOff);
	m_scOff->setOffset(0);
	m_scOff->setScale(1);
	m_signalPath->setEnabled(false);

	m_scaleAvailable = true; // query from GRIIOFloatChannel;
	m_unit = "Volts";	 // query from GRIIOFloatChannel;

	setDevice(dev);

	m_channelName = m_grch->getChannelName();
	auto plot = plotAddon->plot();

	m_plotAxis = new PlotAxis(yPlotAxisPosition, plot, pen, this);
	m_plotCh = new PlotChannel(m_channelName, pen, plot->xAxis(), m_plotAxis, this);

	m_plotAxisHandle = new PlotAxisHandle(plot, m_plotAxis);
	m_plotAxisHandle->handle()->setBarVisibility(BarVisibility::ON_HOVER);
	m_plotAxisHandle->handle()->setColor(pen.color());
	m_plotAxisHandle->handle()->setHandlePos(yPlotAxisHandle == QwtAxis::YLeft ? HandlePos::NORTH_OR_WEST
										   : HandlePos::SOUTH_OR_EAST);
	connect(m_plotAxisHandle, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		double min = m_plotAxis->min() - pos;
		double max = m_plotAxis->max() - pos;
		m_plotAxis->setInterval(min, max);
		plot->plot()->replot();
	});

	m_plotCh->setHandle(m_plotAxisHandle);
	plot->addPlotAxisHandle(m_plotAxisHandle);
	plot->addPlotChannel(m_plotCh);
	m_plotCh->xAxis()->setUnits("s");

	m_measureMgr = new TimeMeasureManager(this);
	m_measureMgr->initMeasure(m_pen);
	m_measureMgr->getModel()->setAdcBitCount(grch()->getFmt()->bits);

	widget = createMenu();
	m_sampleRateAvailable = m_grch->sampleRateAvailable();
}

GRTimeChannelAddon::~GRTimeChannelAddon() {}

QWidget *GRTimeChannelAddon::createYAxisMenu(QWidget *parent)
{
	MenuSectionWidget *yaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yaxis = new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, yaxiscontainer);

	// Y-MODE
	m_ymodeCb = new MenuCombo("YMODE", yaxis);
	auto cb = m_ymodeCb->combo();
	cb->addItem("ADC Counts", YMODE_COUNT);
	cb->addItem("% Full Scale", YMODE_FS);

	if(m_scaleAvailable) {
		cb->addItem(m_unit, YMODE_SCALE);
	}
	m_plotAxis->setUnits("V");
	m_yCtrl = new MenuPlotAxisRangeControl(m_plotAxis, yaxis);

	m_autoscaleBtn = new MenuOnOffSwitch(tr("AUTOSCALE"), yaxis, false);
	m_autoscale = new PlotAutoscaler(this);
	m_autoscale->addChannels(m_plotCh);

	connect(m_autoscale, &PlotAutoscaler::newMin, m_yCtrl, &MenuPlotAxisRangeControl::setMin);
	connect(m_autoscale, &PlotAutoscaler::newMax, m_yCtrl, &MenuPlotAxisRangeControl::setMax);

	connect(m_autoscaleBtn->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yCtrl->setEnabled(!b);
		m_autoscaleEnabled = b;
		toggleAutoScale();
	});

	yaxis->contentLayout()->addWidget(m_autoscaleBtn);
	yaxis->contentLayout()->addWidget(m_yCtrl);
	yaxis->contentLayout()->addWidget(m_ymodeCb);

	yaxiscontainer->contentLayout()->addWidget(yaxis);

	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		auto mode = cb->itemData(idx).toInt();
		setYMode(static_cast<YMode>(mode));
	});

	return yaxiscontainer;
}

QPushButton *GRTimeChannelAddon::createSnapshotButton(QWidget *parent)
{
	QPushButton *snapBtn = new QPushButton("Snapshot", parent);
	StyleHelper::BlueButton(snapBtn);

	connect(snapBtn, &QPushButton::clicked, this, [=]() {
		std::vector<float> x, y;
		auto data = m_plotCh->curve()->data();
		for(int i = 0; i < data->size(); i++) {
			x.push_back(data->sample(i).x());
			y.push_back(data->sample(i).y());
		}
		SnapshotProvider::SnapshotRecipe rec{x, y, m_channelName};
		Q_EMIT addNewSnapshot(rec);
	});

	snapBtn->setEnabled(false);
	return snapBtn;
}

QWidget *GRTimeChannelAddon::createMenu(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout();

	QScrollArea *scroll = new QScrollArea(parent);
	QWidget *wScroll = new QWidget(scroll);
	QVBoxLayout *layScroll = new QVBoxLayout();
	layScroll->setMargin(0);
	layScroll->setSpacing(10);

	wScroll->setLayout(layScroll);
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
	layScroll->addWidget(yaxismenu);
	layScroll->addWidget(curvemenu);
	layScroll->addWidget(attrmenu);
	layScroll->addWidget(measuremenu);
	layScroll->addWidget(m_snapBtn);

	layScroll->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	return w;
}

QWidget *GRTimeChannelAddon::createAttrMenu(QWidget *parent)
{
	MenuSectionWidget *attrcontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *attr =
		new MenuCollapseSection("ATTRIBUTES", MenuCollapseSection::MHCW_NONE, attrcontainer);
	QList<IIOWidget *> attrWidgets = IIOWidgetBuilder().channel(grch()->channel()).buildAll();

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

void GRTimeChannelAddon::setDevice(GRDeviceAddon *d)
{
	m_dev = d;
	d->registerChannel(this);
}

GRDeviceAddon *GRTimeChannelAddon::getDevice() { return m_dev; }

void GRTimeChannelAddon::enable()
{
	m_signalPath->setEnabled(true);
	TimeChannelAddon::enable();
}

void GRTimeChannelAddon::disable()
{
	m_signalPath->setEnabled(false);
	TimeChannelAddon::disable();
}

void GRTimeChannelAddon::onStart()
{
	m_running = true;
	m_measureMgr->getModel()->setSampleRate(m_plotAddon->sampleRate());
	toggleAutoScale();
}

void GRTimeChannelAddon::onStop()
{
	m_running = false;
	toggleAutoScale();
	if(m_autoscaleEnabled) {
		m_autoscale->autoscale();
	}
}

void GRTimeChannelAddon::toggleAutoScale()
{
	if(m_running && m_autoscaleEnabled) {
		m_autoscale->start();
	} else {
		m_autoscale->stop();
	}
}

void GRTimeChannelAddon::setYMode(YMode mode)
{
	double scale = 1;
	double offset = 0;
	double ymin = -1;
	double ymax = 1;
	const iio_data_format *fmt = m_grch->getFmt();

	switch(mode) {
	case YMODE_COUNT:
		scale = 1;
		if(fmt->is_signed) {
			ymin = -(float)((int64_t)1 << (fmt->bits - 1));
			ymax = (float)((int64_t)1 << (fmt->bits - 1));
		} else {
			ymin = 0;
			ymax = 1 << (fmt->bits);
		}
		m_plotCh->yAxis()->setUnits("Counts");
		break;
	case YMODE_FS:
		scale = 1.0 / ((float)((uint64_t)1 << fmt->bits));
		if(fmt->is_signed) {
			ymin = -0.5;
			ymax = 0.5;
		} else {
			ymin = 0;
			ymax = 1;
		}
		m_plotCh->yAxis()->setUnits("");
		break;
	case YMODE_SCALE:
		m_plotCh->yAxis()->setUnits(m_unit);
		break;
	default:
		break;
	}

	m_yCtrl->setMin(ymin);
	m_yCtrl->setMax(ymax);
	m_scOff->setScale(scale);
	m_scOff->setOffset(offset);
}

void GRTimeChannelAddon::setSingleYMode(bool b)
{
	if(b) {
		m_plotCh->curve()->setYAxis(m_plotAddon->plot()->yAxis()->axisId()); // get default axis
	} else {
		m_plotCh->curve()->setYAxis(m_plotAxis->axisId()); // set it's own axis
	}
	m_plotAxisHandle->handle()->setVisible(!b);
	m_yCtrl->setEnabled(!b);
	m_autoscaleBtn->onOffswitch()->setChecked(false);
	m_autoscaleBtn->setEnabled(!b);
}

bool GRTimeChannelAddon::sampleRateAvailable() { return m_sampleRateAvailable; }

double GRTimeChannelAddon::sampleRate() { return m_grch->readSampleRate(); }

MeasureManagerInterface *GRTimeChannelAddon::getMeasureManager() { return m_measureMgr; }

GRIIOFloatChannelSrc *GRTimeChannelAddon::grch() const { return m_grch; }

void GRTimeChannelAddon::onInit()
{
	// Defaults
	/*m_yCtrl->setMin(-1.0);
	m_yCtrl->setMax(1.0);*/
	auto v = Preferences::get("adc_default_y_mode").toInt();
	m_ymodeCb->combo()->setCurrentIndex(v);
	setYMode(static_cast<YMode>(v));
}

void GRTimeChannelAddon::onDeinit() {}

void GRTimeChannelAddon::preFlowBuild() {}
void GRTimeChannelAddon::onNewData(const float *xData, const float *yData, int size)
{
	auto model = m_measureMgr->getModel();
	model->setDataSource(yData, size);
	model->measure();
	m_snapBtn->setEnabled(true);
}

GRSignalPath *GRTimeChannelAddon::signalPath() const { return m_signalPath; }

ImportChannelAddon::ImportChannelAddon(QString name, PlotAddon *plotAddon, QPen pen, QObject *parent)
	: TimeChannelAddon(name, plotAddon, pen, parent)
{

	int yPlotAxisPosition = Preferences::get("adc_plot_yaxis_label_position").toInt();
	int yPlotAxisHandle = Preferences::get("adc_plot_yaxis_handle_position").toInt();
	auto plot = plotAddon->plot();
	;

	m_plotAxis = new PlotAxis(yPlotAxisPosition, plot, pen, this);
	m_plotCh = new PlotChannel(m_channelName, pen, plot->xAxis(), m_plotAxis, this);

	m_plotAxisHandle = new PlotAxisHandle(plot, m_plotAxis);
	m_plotAxisHandle->handle()->setBarVisibility(BarVisibility::ON_HOVER);
	m_plotAxisHandle->handle()->setColor(pen.color());
	m_plotAxisHandle->handle()->setHandlePos(yPlotAxisHandle == QwtAxis::YLeft ? HandlePos::NORTH_OR_WEST
										   : HandlePos::SOUTH_OR_EAST);
	connect(m_plotAxisHandle, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		double min = m_plotAxis->min() - pos;
		double max = m_plotAxis->max() - pos;
		m_plotAxis->setInterval(min, max);
		plot->plot()->replot();
	});

	m_plotCh->setHandle(m_plotAxisHandle);
	plot->addPlotAxisHandle(m_plotAxisHandle);
	plot->addPlotChannel(m_plotCh);
	m_plotCh->xAxis()->setUnits("s");

	m_measureMgr = new TimeMeasureManager(this);
	m_measureMgr->initMeasure(m_pen);

	widget = createMenu();
}

ImportChannelAddon::~ImportChannelAddon() {}

MeasureManagerInterface *ImportChannelAddon::getMeasureManager() { return m_measureMgr; }
void ImportChannelAddon::setSingleYMode(bool b)
{
	if(b) {
		m_plotCh->curve()->setYAxis(m_plotAddon->plot()->yAxis()->axisId()); // get default axis
	} else {
		m_plotCh->curve()->setYAxis(m_plotAxis->axisId()); // set it's own axis
	}
	m_plotAxisHandle->handle()->setVisible(!b);
	m_yCtrl->setEnabled(!b);
}

bool ImportChannelAddon::sampleRateAvailable() { return false; }

double ImportChannelAddon::sampleRate() { return 1; }

void ImportChannelAddon::setData(std::vector<float> x, std::vector<float> y)
{
	m_plotCh->curve()->setSamples(x.data(), y.data(), x.size());
	m_measureMgr->getModel()->setDataSource(y.data(), y.size());
	m_measureMgr->getModel()->measure();
}

QWidget *ImportChannelAddon::createMenu(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);

	QScrollArea *scroll = new QScrollArea(parent);
	QWidget *wScroll = new QWidget(scroll);
	QVBoxLayout *layScroll = new QVBoxLayout(wScroll);
	layScroll->setMargin(0);
	layScroll->setSpacing(10);

	wScroll->setLayout(layScroll);
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
	QWidget *measuremenu = m_measureMgr->createMeasurementMenu(w);
	QWidget *forgetBtn = createForgetButton(w);

	lay->addWidget(header);
	lay->addWidget(scroll);
	layScroll->addWidget(yaxismenu);
	layScroll->addWidget(curvemenu);
	layScroll->addWidget(measuremenu);
	layScroll->addWidget(forgetBtn);

	layScroll->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	return w;
}

QWidget *ImportChannelAddon::createYAxisMenu(QWidget *parent)
{
	MenuSectionWidget *yaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yaxis = new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, yaxiscontainer);

	m_yCtrl = new MenuPlotAxisRangeControl(m_plotAxis, yaxis);

	yaxis->contentLayout()->addWidget(m_yCtrl);
	yaxiscontainer->contentLayout()->addWidget(yaxis);

	return yaxiscontainer;
}

QPushButton *ImportChannelAddon::createForgetButton(QWidget *parent)
{
	QPushButton *btn = new QPushButton("Delete", parent);
	StyleHelper::BlueButton(btn);

	connect(btn, &QPushButton::clicked, this, [=]() { Q_EMIT requestDeleteChannel(this); });

	return btn;
}

void ImportChannelAddon::onDeinit() {}

#include "moc_grtimechanneladdon.cpp"
