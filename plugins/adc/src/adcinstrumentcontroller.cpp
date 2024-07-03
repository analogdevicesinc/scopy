#include "adcinstrumentcontroller.h"
#include "timeplotcomponent.h"
#include "adcinstrument.h"
#include "grdevicecomponent.h"
#include "grtimechannelcomponent.h"
#include "importchannelcomponent.h"
#include "grtimesinkcomponent.h"

#include <pluginbase/preferences.h>
#include "interfaces.h"

Q_LOGGING_CATEGORY(CAT_TIMEPLOT_PROXY, "TimePlotProxy")
using namespace scopy;
using namespace scopy::adc;

ADCInstrumentController::ADCInstrumentController(QString name, AcqTreeNode *tree, QObject *parent)
	: QObject(parent)
	, m_refreshTimerRunning(false)
	, m_tool(nullptr)
	, m_plotComponentManager(nullptr)
	, m_timePlotSettingsComponent(nullptr)
	, m_cursorComponent(nullptr)
	, m_measureComponent(nullptr)
	, currentCategory("time")
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
	connect(
		m_fw, &QFutureWatcher<void>::finished, this,
		[=]() {
			update();
			if(m_refreshTimerRunning)
				m_plotTimer->start();
		},
		Qt::QueuedConnection);
}

ADCInstrumentController::~ADCInstrumentController() {}

ChannelIdProvider *ADCInstrumentController::getChannelIdProvider() { return chIdP; }

ToolComponent *ADCInstrumentController::getPlotAddon() { return (ToolComponent *)m_plotComponentManager; }

ToolComponent *ADCInstrumentController::getPlotSettings() { return (ToolComponent *)m_timePlotSettingsComponent; }

QWidget *ADCInstrumentController::getInstrument() { return (QWidget *)(m_tool); }

void ADCInstrumentController::setInstrument(QWidget *t)
{
	ADCInstrument *ai = dynamic_cast<ADCInstrument *>(t);
	Q_ASSERT(ai);
	m_tool = ai;
}

void ADCInstrumentController::stop() {
	qInfo()<<"Stopping "<<m_name;
	Q_EMIT requestStop();
	qInfo()<<"Stopped "<<m_name;
}

void ADCInstrumentController::init()
{
	ToolTemplate *toolLayout = m_tool->getToolTemplate();

	m_plotComponentManager = new TimePlotManager(m_name + "_time", m_tool);
	addComponent(m_plotComponentManager);
	m_timePlotSettingsComponent = new TimePlotManagerSettings(m_plotComponentManager);
	addComponent(m_timePlotSettingsComponent);

	uint32_t tmp;
	tmp = m_plotComponentManager->addPlot("Acceleration 1");
	m_timePlotSettingsComponent->addPlot(m_plotComponentManager->plot(tmp));
	tmp = m_plotComponentManager->addPlot("Inertial movement 2");
	m_timePlotSettingsComponent->addPlot(m_plotComponentManager->plot(tmp));

	// m_cursorComponent = new CursorComponent(m_plotComponentManager, m_tool->getToolTemplate(), this);
	// addComponent(m_cursorComponent);

	m_measureComponent = new MeasureComponent(m_tool->getToolTemplate(), m_plotComponentManager, this);
	// m_measureComponent->addPlotComponent(m_plotComponentManager);

	addComponent(m_measureComponent);

	plotStack = new MapStackedWidget(m_tool);
	toolLayout->addWidgetToCentralContainerHelper(plotStack);

	plotStack->add("time", m_plotComponentManager);
	toolLayout->rightStack()->add(m_tool->settingsMenuId, m_timePlotSettingsComponent);

	for(auto c : qAsConst(m_components)) {
		c->onInit();
	}

	for(auto *node : m_tree->bfs()) {
		addChannel(node);
	}

	connect(m_tool, &ADCInstrument::setSingleShot, this, &ADCInstrumentController::setSingleShot);

	m_otherCMCB = new CollapsableMenuControlButton(m_tool->vcm());
	m_otherCMCB->getControlBtn()->button()->setVisible(false);
	m_otherCMCB->getControlBtn()->setName("Other");
	m_tool->vcm()->addEnd(m_otherCMCB);
}

void ADCInstrumentController::deinit()
{
	for(auto c : qAsConst(m_components)) {
		c->onDeinit();
	}
}

void ADCInstrumentController::onStart()
{
	ResourceManager::open("adc",this);
	for(auto c : qAsConst(m_components)) {
		if(c->enabled()) {
			c->onStart();
		}
	}
	dynamic_cast<GRTimeSinkComponent*>(m_dataProvider)->onStart();
	startUpdates();

}

void ADCInstrumentController::onStop()
{
	dynamic_cast<GRTimeSinkComponent*>(m_dataProvider)->onStop();
	for(int idx = m_components.size() - 1 ; idx >= 0;idx--) {
		auto c = m_components[idx];
		c->onStop();
	}
	stopUpdates();
	ResourceManager::close("adc");
}

void ADCInstrumentController::stopUpdates()
{
	qInfo(CAT_TIMEPLOT_PROXY) << "Stopped plotting";
	m_refreshTimerRunning = false;

	m_refillFuture.cancel();
	m_plotTimer->stop();
}

void ADCInstrumentController::startUpdates()
{
	qInfo(CAT_TIMEPLOT_PROXY) << "Start plotting";
	updateFrameRate();
	m_refreshTimerRunning = true;
	update();
	m_plotTimer->start();
}

void ADCInstrumentController::setSingleShot(bool b)
{
	m_dataProvider->setSingleShot(b);
}

void ADCInstrumentController::updateData()
{
	m_refillFuture = QtConcurrent::run([=]() {
		m_dataProvider->updateData();
	});
	m_fw->setFuture(m_refillFuture);
}

void ADCInstrumentController::update()
{
	m_dataProvider->setData(false);
	if(m_dataProvider->finished()) {
		Q_EMIT requestStopLater();
	}
	m_plotComponentManager->replot();
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

void ADCInstrumentController::addChannel(AcqTreeNode *node)
{
	qInfo() << node->name();

	if(dynamic_cast<GRTopBlockNode *>(node) != nullptr) {
		GRTopBlockNode *grtbn = dynamic_cast<GRTopBlockNode *>(node);
		GRTimeSinkComponent *c = new GRTimeSinkComponent(m_name + "_time", grtbn, this);
		m_acqNodeComponentMap[grtbn] = (c);
		//addComponent(c);

		m_dataProvider = c;
		c->onInit();

		connect(m_timePlotSettingsComponent, &TimePlotManagerSettings::bufferSizeChanged, c,
			&GRTimeSinkComponent::setBufferSize);
		connect(m_timePlotSettingsComponent, &TimePlotManagerSettings::plotSizeChanged, c,
			&GRTimeSinkComponent::setPlotSize);
		connect(m_timePlotSettingsComponent, &TimePlotManagerSettings::sampleRateChanged, c,
			&GRTimeSinkComponent::setSampleRate);
		connect(m_timePlotSettingsComponent, &TimePlotManagerSettings::rollingModeChanged, c,
			&GRTimeSinkComponent::setRollingMode);
	}

	if(dynamic_cast<GRIIODeviceSourceNode *>(node) != nullptr) {
		GRIIODeviceSourceNode *griiodsn = dynamic_cast<GRIIODeviceSourceNode *>(node);
		GRDeviceComponent *d = new GRDeviceComponent(griiodsn);
		addComponent(d);
		m_tool->addDevice(d->ctrl(), d);

		m_acqNodeComponentMap[griiodsn] = (d);
		m_timePlotSettingsComponent->addSampleRateProvider(d);
		addComponent(d);

		connect(m_timePlotSettingsComponent, &TimePlotManagerSettings::bufferSizeChanged, d,
			&GRDeviceComponent::setBufferSize);
	}

	if(dynamic_cast<GRIIOFloatChannelNode *>(node) != nullptr) {
		int idx = chIdP->next();
		GRIIOFloatChannelNode *griiofcn = dynamic_cast<GRIIOFloatChannelNode *>(node);
		GRTimeSinkComponent *grtsc =
			dynamic_cast<GRTimeSinkComponent *>(m_acqNodeComponentMap[griiofcn->top()]);
		GRTimeChannelComponent *c =
			new GRTimeChannelComponent(griiofcn, m_plotComponentManager->plot(0), grtsc, chIdP->pen(idx));
		Q_ASSERT(grtsc);

		m_plotComponentManager->addChannel(c);
		QWidget *ww = m_plotComponentManager->plotCombo(c);
		c->menu()->add(ww, "plot", gui::MenuWidget::MA_BOTTOMFIRST);

		/*** This is a bit of a mess because CollapsableMenuControlButton is not a MenuControlButton ***/

		CompositeWidget *cw = nullptr;
		GRIIODeviceSourceNode *w = dynamic_cast<GRIIODeviceSourceNode *>(griiofcn->treeParent());
		GRDeviceComponent *dc = dynamic_cast<GRDeviceComponent *>(m_acqNodeComponentMap[w]);
		if(w) {
			cw = dc->ctrl();
		}
		if(!cw) {
			cw = m_tool->vcm();
		}
		m_acqNodeComponentMap[griiofcn] = c;

		/*** End of mess ***/

		m_tool->addChannel(c->ctrl(), c, cw);

		connect(c->ctrl(), &QAbstractButton::clicked, this,
			[=]() { m_plotComponentManager->selectChannel(c); });

		grtsc->addChannel(c);			    // For matching Sink To Channels
		dc->addChannel(c);			    // used for sample rate computation
		m_timePlotSettingsComponent->addChannel(c); // SingleY/etc

		addComponent(c);
		setupChannelMeasurement(m_plotComponentManager, c);
	}

	if(dynamic_cast<ImportFloatChannelNode *>(node) != nullptr) {
		int idx = chIdP->next();
		ImportFloatChannelNode *ifcn = dynamic_cast<ImportFloatChannelNode *>(node);
		ImportChannelComponent *c = new ImportChannelComponent(ifcn, chIdP->pen(idx));

		m_plotComponentManager->addChannel(c);
		c->menu()->add(m_plotComponentManager->plotCombo(c), "plot", gui::MenuWidget::MA_BOTTOMFIRST);

		CompositeWidget *cw = m_otherCMCB;
		m_acqNodeComponentMap[ifcn] = c;
		m_tool->addChannel(c->ctrl(), c, cw);

		connect(c->ctrl(), &QAbstractButton::clicked, this,
			[=]() { m_plotComponentManager->selectChannel(c); });

		c->ctrl()->animateClick();

		m_timePlotSettingsComponent->addChannel(c); // SingleY/etc

		addComponent(c);
		setupChannelMeasurement(m_plotComponentManager, c);
	}
	m_plotComponentManager->replot();
}

void ADCInstrumentController::removeChannel(AcqTreeNode *node)
{
	if(dynamic_cast<ImportFloatChannelNode *>(node) != nullptr) {
		ImportFloatChannelNode *ifcn = dynamic_cast<ImportFloatChannelNode *>(node);
		ImportChannelComponent *c = dynamic_cast<ImportChannelComponent *>(m_acqNodeComponentMap[ifcn]);

		m_otherCMCB->remove(c->ctrl());
		m_plotComponentManager->removeChannel(c);
		m_timePlotSettingsComponent->removeChannel(c);
		removeComponent(c);
		delete c;
	}
	m_plotComponentManager->replot();
}

/*void ADCInstrumentController::createSnapshotChannel(SnapshotProvider::SnapshotRecipe rec)
{
	//	proxy->getChannelAddons().append(new ch)
	qInfo() << "Creating snapshot from recipe" << rec.name;

	int idx = chIdP->next();
	ImportChannelAddon *ch = new ImportChannelAddon("REF-" + rec.name + "-" + QString::number(idx), plotAddon,
							chidp->pen(idx), this);
	proxy->addChannelAddon(ch);
	ch->setData(rec.x, rec.y);
	auto btn = addChannel(ch, vcm);
	vcm->add(btn);
	ch->onInit();
	btn->animateClick(1);
}*/

void ADCInstrumentController::setupChannelMeasurement(TimePlotManager *c, ChannelComponent *ch)
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
		connect(measureSettings, &MeasurementSettings::toggleAllMeasurements, [=](bool b) {
			measurePanel->setInhibitUpdates(true);
			chMeasureManager->toggleAllMeasurement(b);
			measurePanel->setInhibitUpdates(false);
		});
		connect(measureSettings, &MeasurementSettings::toggleAllStats,
			[=](bool b) { chMeasureManager->toggleAllStats(b); });
		connect(chMeasureManager, &MeasureManagerInterface::enableStat, statsPanel, &StatsPanel::addStat);
		connect(chMeasureManager, &MeasureManagerInterface::disableStat, statsPanel, &StatsPanel::removeStat);
	}
}


uint32_t SyncController::bufferSize() const
{
	return m_bufferSize;
}

void SyncController::setBufferSize(uint32_t newBufferSize)
{
	if (m_bufferSize == newBufferSize)
		return;
	m_bufferSize = newBufferSize;
	emit bufferSizeChanged();
}

bool SyncController::singleShot() const
{
	return m_singleShot;
}

void SyncController::setSingleShot(bool newSingleShot)
{
	if (m_singleShot == newSingleShot)
		return;
	m_singleShot = newSingleShot;
	emit singleShotChanged();
}
