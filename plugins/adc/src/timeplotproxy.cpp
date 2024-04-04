#include "timeplotproxy.h"
#include "adcplugin.h"
#include "plotcomponent.h"
#include "adctimeinstrument.h"
#include "grdevicecomponent.h"
#include "grtimechannelcomponent.h"
#include "grtimesinkcomponent.h"
#include <pluginbase/preferences.h>

Q_LOGGING_CATEGORY(CAT_TIMEPLOT_PROXY, "TimePlotProxy")
using namespace scopy;
using namespace scopy::adc;

TimePlotProxy::TimePlotProxy(QString name, AcqTreeNode * tree, QObject *parent)
	: QObject(parent)
	, m_refreshTimerRunning(false)
	,m_tool(nullptr)
	,m_plotComponent(nullptr)
	,m_plotSettingsComponent(nullptr)
	,m_cursorComponent(nullptr)
	,m_measureComponent(nullptr)
{
	chIdP = new ChannelIdProvider(this);
	m_tree = tree;
	m_name = name;

	Preferences *p = Preferences::GetInstance();

	m_plotTimer = new QTimer(this);
	m_plotTimer->setSingleShot(true);
	connect(m_plotTimer, &QTimer::timeout, this, &TimePlotProxy::updateData);
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

TimePlotProxy::~TimePlotProxy() {}

ChannelIdProvider *TimePlotProxy::getChannelIdProvider() { return chIdP; }

void TimePlotProxy::setInstrument(ADCTimeInstrument *t) {
	m_tool = t;
}

ToolComponent *TimePlotProxy::getPlotAddon() { return (ToolComponent *)m_plotComponent; }

ToolComponent *TimePlotProxy::getPlotSettings() { return (ToolComponent *)m_plotSettingsComponent; }

QWidget *TimePlotProxy::getInstrument() { return (QWidget *)(m_tool); }

void TimePlotProxy::setInstrument(QWidget *t)
{
	ADCTimeInstrument *ai = dynamic_cast<ADCTimeInstrument *>(t);
	Q_ASSERT(ai);
	m_tool = ai;
}

void TimePlotProxy::init()
{
	ToolTemplate *toolLayout = m_tool->getToolTemplate();
	m_plotComponent = new PlotComponent(m_name, m_tool);
	addComponent(m_plotComponent);

	m_plotSettingsComponent = new TimePlotSettingsComponent(m_plotComponent);
	addComponent(m_plotSettingsComponent);

	m_cursorComponent = new CursorComponent(m_plotComponent, m_tool->getToolTemplate(), this);
	addComponent(m_cursorComponent);

	m_measureComponent = new MeasureComponent(m_tool->getToolTemplate(), this);
	addComponent(m_measureComponent);



	connect(m_plotSettingsComponent, &TimePlotSettingsComponent::sampleRateChanged, this,
		[=](double v) {
			for(auto c : qAsConst(m_components)) {
				SampleRateUser *comp =  dynamic_cast<SampleRateUser *>(c);
				if(comp) {
					comp->setSampleRate(v);
				}
			}
		});

	connect(m_plotSettingsComponent, &TimePlotSettingsComponent::rollingModeChanged, this,
		[=](bool v) {
			for(auto c : qAsConst(m_components)) {
				RollingModeUser *comp =  dynamic_cast<RollingModeUser *>(c);
				if(comp) {
					comp->setRollingMode(v);
				}
			}
		});

	connect(m_plotSettingsComponent, &TimePlotSettingsComponent::plotSizeChanged, this,
		[=](uint32_t v) {
			for(auto c : qAsConst(m_components)) {
				PlotSizeUser *comp =  dynamic_cast<PlotSizeUser *>(c);
				if(comp) {
					comp->setPlotSize(v);
				}
			}
		});

	connect(m_plotSettingsComponent, &TimePlotSettingsComponent::singleYModeChanged, this,
		[=](bool v) {
			for(auto c : qAsConst(m_components)) {
				SingleYModeUser *comp =  dynamic_cast<SingleYModeUser *>(c);
				if(comp) {
					comp->setSingleYMode(v);
				}
			}
		});

	connect(m_plotSettingsComponent, &TimePlotSettingsComponent::bufferSizeChanged, this,
		[=](uint32_t v) {
			for(auto c : qAsConst(m_components)) {
				BufferSizeUser *comp =  dynamic_cast<BufferSizeUser *>(c);
				if(comp) {
					comp->setBufferSize(v);
				}
			}
		});

	toolLayout->addWidgetToCentralContainerHelper(m_plotComponent);
	toolLayout->rightStack()->add(m_tool->settingsMenuId, m_plotSettingsComponent);

	for(auto c : qAsConst(m_components)) {
		c->onInit();
	}

	for(auto *node : m_tree->bfs()) {
		addChannel(node);
	}

	connect(this, &TimePlotProxy::requestStop, m_tool, &ADCTimeInstrument::stop, Qt::QueuedConnection);
	connect(m_tool, &ADCTimeInstrument::setSingleShot, this, &TimePlotProxy::setSingleShot);

}

void TimePlotProxy::deinit() {
	for(auto c : qAsConst(m_components)) {
		c->onDeinit();
	}
}

void TimePlotProxy::onStart() {
	for(auto c : qAsConst(m_components)) {
		c->onStart();
	}
	startUpdates();
}

void TimePlotProxy::onStop() {
	for(auto c : qAsConst(m_components)) {
		c->onStop();
	}
	stopUpdates();
}


void TimePlotProxy::stopUpdates() {
	qInfo(CAT_TIMEPLOT_PROXY) << "Stopped plotting";
	m_refreshTimerRunning = false;

	m_refillFuture.cancel();
	m_plotTimer->stop();
}

void TimePlotProxy::startUpdates() {
	qInfo(CAT_TIMEPLOT_PROXY) << "Start plotting";
	updateFrameRate();
	m_refreshTimerRunning = true;
	update();
	m_plotTimer->start();
}

void TimePlotProxy::setSingleShot(bool b)
{
	for(ToolComponent *c : qAsConst(m_components)) {
		if(dynamic_cast<DataProvider*>(c)) {
			DataProvider *dp = dynamic_cast<DataProvider*>(c);
			dp->setSingleShot(b);
		}
	}
}

void TimePlotProxy::updateData() {
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

void TimePlotProxy::update()
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
	m_plotComponent->replot();
}

void TimePlotProxy::handlePreferences(QString key, QVariant v)
{
	if(key == "general_plot_target_fps") {
		updateFrameRate();
	}
}

void TimePlotProxy::updateFrameRate()
{
	Preferences *p = Preferences::GetInstance();
	double framerate = p->get("general_plot_target_fps").toDouble();
	setFrameRate(framerate);
}

void TimePlotProxy::setFrameRate(double val)
{
	int timeout = (1.0 / val) * 1000;
	m_plotTimer->setInterval(timeout);
}

void TimePlotProxy::addChannel(AcqTreeNode *node) {
	qInfo()<<node->name();

	if(dynamic_cast<GRTopBlockNode*>(node) != nullptr) {
		GRTopBlockNode* grtbn = dynamic_cast<GRTopBlockNode*>(node);
		GRTimeSinkComponent *c = new GRTimeSinkComponent(m_name, grtbn, this);

		m_acqNodeComponentMap[grtbn] = c;		
		addComponent(c);
	}

	if(dynamic_cast<GRIIODeviceSourceNode*>(node) != nullptr) {
		GRIIODeviceSourceNode* griiodsn = dynamic_cast<GRIIODeviceSourceNode*>(node);
		GRDeviceComponent *d = new GRDeviceComponent(griiodsn);
		addComponent(d);
		m_tool->addDevice(d->ctrl(), d);

		m_acqNodeComponentMap[griiodsn] = d;
		addComponent(d);

		SampleRateProvider *s = dynamic_cast<SampleRateProvider*>(d);
		if(s) {
			m_plotSettingsComponent->addSampleRateProvider(s);
		}

	}

	if(dynamic_cast<GRIIOFloatChannelNode*>(node) != nullptr) {
		int idx = chIdP->next();
		GRIIOFloatChannelNode* griiofcn = dynamic_cast<GRIIOFloatChannelNode*>(node);
		GRTimeChannelComponent *c = new GRTimeChannelComponent(griiofcn, m_plotComponent, chIdP->pen(idx));

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
		m_plotSettingsComponent->addChannel(c); // SingleY/etc

		SampleRateProvider *s = dynamic_cast<SampleRateProvider*>(c); // SampleRate Computation
		if(s) {
			m_plotSettingsComponent->addSampleRateProvider(s);
		}

		addComponent(c);
		setupChannelMeasurement(c);

	}
}

void TimePlotProxy::setupChannelMeasurement(ChannelComponent *ch)
{
	auto chMeasureableChannel = dynamic_cast<MeasurementProvider *>(ch);
	if(!chMeasureableChannel)
		return;
	auto chMeasureManager = chMeasureableChannel->getMeasureManager();
	if(!chMeasureManager)
		return;
	if(m_measureComponent) {
		auto measureSettings = m_measureComponent->measureSettings();
		auto measurePanel = m_measureComponent->measurePanel();
		auto statsPanel = m_measureComponent->statsPanel();
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

void TimePlotProxy::removeChannel(AcqTreeNode *node) {
	// removeComponent(node);
}


