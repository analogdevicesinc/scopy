#include "adcinstrumentcontroller.h"
#include "adcplugin.h"
#include "plotcomponent.h"
#include "adcinstrument.h"
#include "grdevicecomponent.h"
#include "grtimechannelcomponent.h"
#include "grtimesinkcomponent.h"
#include <pluginbase/preferences.h>
#include "interfaces.h"


Q_LOGGING_CATEGORY(CAT_TIMEPLOT_PROXY, "TimePlotProxy")
using namespace scopy;
using namespace scopy::adc;

ADCInstrumentController::ADCInstrumentController(QString name, AcqTreeNode * tree, QObject *parent)
	: QObject(parent)
	, m_refreshTimerRunning(false)
	,m_tool(nullptr)
	,m_timePlotComponent(nullptr)
	,m_timePlotSettingsComponent(nullptr)
	,m_cursorComponent(nullptr)
	,m_measureComponent(nullptr)
{
	chIdP = new ChannelIdProvider(this);
	m_tree = tree;
	m_name = name;

	Preferences *p = Preferences::GetInstance();

	m_plotTimer = new QTimer(this);
	m_plotTimer->setSingleShot(true);
	connect(m_plotTimer, &QTimer::timeout, this, &ADCInstrumentController::updateData);
	connect(p, SIGNAL(preferenceChanged(QString, QVariant)), this, SLOT(handlePreferences(QString, QVariant)));

	m_fw = new QFutureWatcher<void>(this);
	connect( m_fw, &QFutureWatcher<void>::finished, this,
		[=]() {
			update();
			if(m_refreshTimerRunning)
				m_plotTimer->start();
		},
		Qt::QueuedConnection);
}

ADCInstrumentController::~ADCInstrumentController() {}

ChannelIdProvider *ADCInstrumentController::getChannelIdProvider() { return chIdP; }

ToolComponent *ADCInstrumentController::getPlotAddon() { return (ToolComponent *)m_timePlotComponent; }

ToolComponent *ADCInstrumentController::getPlotSettings() { return (ToolComponent *)m_timePlotSettingsComponent; }

QWidget *ADCInstrumentController::getInstrument() { return (QWidget *)(m_tool); }

void ADCInstrumentController::setInstrument(QWidget *t)
{
	ADCInstrument *ai = dynamic_cast<ADCInstrument *>(t);
	Q_ASSERT(ai);
	m_tool = ai;
}

void ADCInstrumentController::init()
{
	ToolTemplate *toolLayout = m_tool->getToolTemplate();

	m_timePlotComponent = new PlotComponent(m_name+"_time", m_tool);
	addComponent(m_timePlotComponent);
	m_timePlotSettingsComponent = new TimePlotSettingsComponent(m_timePlotComponent);
	addComponent(m_timePlotSettingsComponent);
	connect(m_tool->getTimeBtn(), &QAbstractButton::pressed, this, [=](){
		enableCategory("time");
	});

	m_fftPlotComponent = new PlotComponent(m_name+"_fft", m_tool);
	addComponent(m_fftPlotComponent);
	/*m_fftPlotSettingsComponent = new FftPlotSettingsComponent(m_fftPlotComponent);
	addComponent(m_fftPlotSettingsComponent);*/
	connect(m_tool->getFftBtn(), &QAbstractButton::pressed, this, [=](){
		enableCategory("fft");
	});

	m_xyPlotComponent = new PlotComponent(m_name+"_xy", m_tool);
	addComponent(m_xyPlotComponent);
	m_xyPlotSettingsComponent = new XyPlotSettingsComponent(m_xyPlotComponent);
	addComponent(m_xyPlotSettingsComponent);
	connect(m_tool->getXyBtn(), &QAbstractButton::pressed, this, [=](){
		enableCategory("xy");
	});

	m_cursorComponent = new CursorComponent(m_timePlotComponent, m_tool->getToolTemplate(), this);
	addComponent(m_cursorComponent);

	m_measureComponent = new MeasureComponent(m_tool->getToolTemplate(), this);
	m_measureComponent->addPlotComponent(m_timePlotComponent);
	m_measureComponent->addPlotComponent(m_fftPlotComponent);
	m_measureComponent->addPlotComponent(m_xyPlotComponent);
	addComponent(m_measureComponent);

	plotStack = new MapStackedWidget(m_tool);
	toolLayout->addWidgetToCentralContainerHelper(plotStack);

	plotStack->add("time",m_timePlotComponent);
	toolLayout->rightStack()->add(m_tool->settingsMenuId+"_time", m_timePlotSettingsComponent);
	plotStack->add("xy", m_xyPlotComponent);
	toolLayout->rightStack()->add(m_tool->settingsMenuId+"_xy", m_xyPlotSettingsComponent);
	plotStack->add("fft", m_fftPlotComponent);
	// toolLayout->rightStack()->add(m_tool->settingsMenuId+"_fft", m_fftPlotSettingsComponent);

	connect(m_timePlotSettingsComponent, &TimePlotSettingsComponent::sampleRateChanged, this,
		[=](double v) {
			for(auto c : qAsConst(m_components)) {
				SampleRateUser *comp =  dynamic_cast<SampleRateUser *>(c);
				if(comp) {
					comp->setSampleRate(v);
				}
			}
		});

	connect(m_timePlotSettingsComponent, &TimePlotSettingsComponent::rollingModeChanged, this,
		[=](bool v) {
			for(auto c : qAsConst(m_components)) {
				RollingModeUser *comp =  dynamic_cast<RollingModeUser *>(c);
				if(comp) {
					comp->setRollingMode(v);
				}
			}
		});

	connect(m_timePlotSettingsComponent, &TimePlotSettingsComponent::plotSizeChanged, this,
		[=](uint32_t v) {
			for(auto c : qAsConst(m_components)) {
				PlotSizeUser *comp =  dynamic_cast<PlotSizeUser *>(c);
				if(comp) {
					comp->setPlotSize(v);
				}
			}
		});

	connect(m_timePlotSettingsComponent, &TimePlotSettingsComponent::singleYModeChanged, this,
		[=](bool v) {
			for(auto c : qAsConst(m_components)) {
				SingleYModeUser *comp =  dynamic_cast<SingleYModeUser *>(c);
				if(comp) {
					comp->setSingleYMode(v);
				}
			}
		});

	connect(m_timePlotSettingsComponent, &TimePlotSettingsComponent::bufferSizeChanged, this,
		[=](uint32_t v) {
			for(auto c : qAsConst(m_components)) {
				BufferSizeUser *comp =  dynamic_cast<BufferSizeUser *>(c);
				if(comp) {
					comp->setBufferSize(v);
				}
			}
		});


	for(auto c : qAsConst(m_components)) {
		c->onInit();
	}

	for(auto *node : m_tree->bfs()) {
		addChannel(node);
	}

	connect(this, &ADCInstrumentController::requestStop, m_tool, &ADCInstrument::stop, Qt::QueuedConnection);
	connect(m_tool, &ADCInstrument::setSingleShot, this, &ADCInstrumentController::setSingleShot);

}

void ADCInstrumentController::deinit() {
	for(auto c : qAsConst(m_components)) {
		c->onDeinit();
	}
}

void ADCInstrumentController::onStart() {
	for(auto c : qAsConst(m_components)) {
		c->onStart();
	}
	startUpdates();
}

void ADCInstrumentController::onStop() {
	for(auto c : qAsConst(m_components)) {
		c->onStop();
	}
	stopUpdates();
}


void ADCInstrumentController::stopUpdates() {
	qInfo(CAT_TIMEPLOT_PROXY) << "Stopped plotting";
	m_refreshTimerRunning = false;

	m_refillFuture.cancel();
	m_plotTimer->stop();
}

void ADCInstrumentController::startUpdates() {
	qInfo(CAT_TIMEPLOT_PROXY) << "Start plotting";
	updateFrameRate();
	m_refreshTimerRunning = true;
	update();
	m_plotTimer->start();
}

void ADCInstrumentController::setSingleShot(bool b)
{
	for(ToolComponent *c : qAsConst(m_components)) {
		if(dynamic_cast<DataProvider*>(c)) {
			DataProvider *dp = dynamic_cast<DataProvider*>(c);
			dp->setSingleShot(b);
		}
	}
}

void ADCInstrumentController::updateData() {
	m_refillFuture = QtConcurrent::run([=]() {
		//		qInfo(CAT_GRTIMEPLOT)<<"UpdateData";
		for(ToolComponent *c : qAsConst(m_components)) {
			if(dynamic_cast<DataProvider*>(c)) {
				DataProvider *dp = dynamic_cast<DataProvider*>(c);
				dp->updateData();
			}
		}
	});
	m_fw->setFuture(m_refillFuture);
}

void ADCInstrumentController::update()
{
	for(ToolComponent *c : qAsConst(m_components)) {
		if(dynamic_cast<DataProvider*>(c)) {
			DataProvider *dp = dynamic_cast<DataProvider*>(c);
			dp->setCurveData();
			if(dp->finished()) {
				Q_EMIT requestStop();
			}
		}
	}
	m_timePlotComponent->replot();
}

void ADCInstrumentController::handlePreferences(QString key, QVariant v)
{
	if(key == "general_plot_target_fps") {
		updateFrameRate();
	}
}

void ADCInstrumentController::updateFrameRate()
{
	Preferences *p = Preferences::GetInstance();
	double framerate = p->get("general_plot_target_fps").toDouble();
	setFrameRate(framerate);
}

void ADCInstrumentController::setFrameRate(double val)
{
	int timeout = (1.0 / val) * 1000;
	m_plotTimer->setInterval(timeout);
}

void ADCInstrumentController::addChannel(AcqTreeNode *node) {
	qInfo()<<node->name();

	if(dynamic_cast<GRTopBlockNode*>(node) != nullptr) {
		GRTopBlockNode* grtbn = dynamic_cast<GRTopBlockNode*>(node);
		GRTimeSinkComponent *c = new GRTimeSinkComponent(m_name, grtbn, this);
		c->category()->append("time");

		m_acqNodeComponentMap[grtbn] = c;		
		addComponent(c);
	}

	if(dynamic_cast<GRIIODeviceSourceNode*>(node) != nullptr) {
		GRIIODeviceSourceNode* griiodsn = dynamic_cast<GRIIODeviceSourceNode*>(node);
		GRDeviceComponent *d = new GRDeviceComponent(griiodsn);
		addComponent(d);
		d->category()->append("time");
		m_tool->addDevice(d->ctrl(), d);

		m_acqNodeComponentMap[griiodsn] = d;
		addComponent(d);

		SampleRateProvider *s = dynamic_cast<SampleRateProvider*>(d);
		if(s) {
			m_timePlotSettingsComponent->addSampleRateProvider(s);
		}

	}

	if(dynamic_cast<GRIIOFloatChannelNode*>(node) != nullptr) {
		int idx = chIdP->next();
		GRIIOFloatChannelNode* griiofcn = dynamic_cast<GRIIOFloatChannelNode*>(node);
		GRTimeChannelComponent *c = new GRTimeChannelComponent(griiofcn, m_timePlotComponent, chIdP->pen(idx));
		c->category()->append("time");

		/*** This is a bit of a mess because CollapsableMenuControlButton is not a MenuControlButton ***/
		CompositeWidget *cw = nullptr;
		GRIIODeviceSourceNode *w = dynamic_cast<GRIIODeviceSourceNode*>(griiofcn->treeParent());
		GRDeviceComponent* dc = dynamic_cast<GRDeviceComponent*>(m_acqNodeComponentMap[w]);
		if(w) {			
			cw = dc->ctrl();
		}
		if(!cw) {
			cw = m_tool->vcm();
		}
		m_acqNodeComponentMap[griiofcn] = c;
		/*** End of mess ***/

		m_tool->addChannel(c->ctrl(), c, cw);

		auto *grNode = m_acqNodeComponentMap[griiofcn->top()];
		GRTimeSinkComponent *grtsc = dynamic_cast<GRTimeSinkComponent*>(grNode);
		Q_ASSERT(grtsc);
		grtsc->addChannel(c); // For matching Sink To Channels
		dc->addChannel(c);    // used for sample rate computation
		m_timePlotSettingsComponent->addChannel(c); // SingleY/etc

		SampleRateProvider *s = dynamic_cast<SampleRateProvider*>(c); // SampleRate Computation
		if(s) {
			m_timePlotSettingsComponent->addSampleRateProvider(s);
		}

		addComponent(c);
		setupChannelMeasurement(m_timePlotComponent, c);

	}
}

void ADCInstrumentController::setupChannelMeasurement(PlotComponent *c ,ChannelComponent *ch)
{
	auto chMeasureableChannel = dynamic_cast<MeasurementProvider *>(ch);
	if(!chMeasureableChannel)
		return;
	auto chMeasureManager = chMeasureableChannel->getMeasureManager();
	if(!chMeasureManager)
		return;
	if(m_measureComponent) {
		auto measureSettings = m_measureComponent->measureSettings();
		auto measurePanel = c->measurePanel();
		auto statsPanel = c->statsPanel();
		connect(chMeasureManager, &MeasureManagerInterface::enableMeasurement, measurePanel,
			&MeasurementsPanel::addMeasurement);
		connect(chMeasureManager, &MeasureManagerInterface::disableMeasurement, measurePanel,
			&MeasurementsPanel::removeMeasurement);
		connect(measureSettings, &MeasurementSettings::toggleAllMeasurements, chMeasureManager,
			&MeasureManagerInterface::toggleAllMeasurement);
		connect(measureSettings, &MeasurementSettings::toggleAllStats, chMeasureManager,
			&MeasureManagerInterface::toggleAllStats);
		connect(chMeasureManager, &MeasureManagerInterface::enableStat, statsPanel, &StatsPanel::addStat);
		connect(chMeasureManager, &MeasureManagerInterface::disableStat, statsPanel, &StatsPanel::removeStat);
	}
}

void ADCInstrumentController::removeChannel(AcqTreeNode *node) {
	// removeComponent(node);
}

void ADCInstrumentController::enableCategory(QString s) {
	QString categoryName = s;
	ToolTemplate *toolLayout = m_tool->getToolTemplate();
	plotStack->show(categoryName);
	toolLayout->requestMenu(m_tool->settingsMenuId+"_"+categoryName);
	// m_tool->vcm()->filter(s);

	for(auto c : qAsConst(m_components)) {
		if(c->category()->contains(categoryName)) {
			c->enable();
		} else {
			c->disable();
		}
	}
	m_tool->restart();
}


