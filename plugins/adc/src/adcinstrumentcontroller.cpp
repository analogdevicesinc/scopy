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

ADCInstrumentController::ADCInstrumentController(ToolMenuEntry *tme, QString name, AcqTreeNode *tree, QObject *parent)
	: QObject(parent)
	, m_refreshTimerRunning(false)
	, m_plotComponentManager(nullptr)
	, m_timePlotSettingsComponent(nullptr)
	, m_cursorComponent(nullptr)
	, m_measureComponent(nullptr)
	, m_started(false)
{	
	chIdP = new ChannelIdProvider(this);
	m_tree = tree;
	m_name = name;

	Preferences *p = Preferences::GetInstance();

	connect(p, SIGNAL(preferenceChanged(QString, QVariant)), this, SLOT(handlePreferences(QString, QVariant)));

	m_plotTimer = new QTimer(this);
	m_plotTimer->setSingleShot(true);
	connect(m_plotTimer, &QTimer::timeout, this, &ADCInstrumentController::update);
	/*m_fw = new QFutureWatcher<void>(this);
	connect(
		m_fw, &QFutureWatcher<void>::finished, this,
		[=]() {
			update();
			if(m_refreshTimerRunning)
				m_plotTimer->start();
		},
		Qt::QueuedConnection);
	*/
	m_ui = new ADCInstrument(tme, nullptr);
	init();
}

ADCInstrumentController::~ADCInstrumentController() {}

ChannelIdProvider *ADCInstrumentController::getChannelIdProvider() { return chIdP; }

ToolComponent *ADCInstrumentController::getPlotAddon() { return (ToolComponent *)m_plotComponentManager; }

ToolComponent *ADCInstrumentController::getPlotSettings() { return (ToolComponent *)m_timePlotSettingsComponent; }

void ADCInstrumentController::init()
{
	ToolTemplate *toolLayout = m_ui->getToolTemplate();

	m_plotComponentManager = new TimePlotManager(m_name + "_time", m_ui);
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

	m_measureComponent = new MeasureComponent(m_ui->getToolTemplate(), m_plotComponentManager, this);
	// m_measureComponent->addPlotComponent(m_plotComponentManager);

	addComponent(m_measureComponent);

	plotStack = new MapStackedWidget(m_ui);
	toolLayout->addWidgetToCentralContainerHelper(plotStack);

	plotStack->add("time", m_plotComponentManager);
	toolLayout->rightStack()->add(m_ui->settingsMenuId, m_timePlotSettingsComponent);

	for(auto c : qAsConst(m_components)) {
		c->onInit();
	}

	for(auto *node : m_tree->bfs()) {
		addChannel(node);
	}

	m_otherCMCB = new CollapsableMenuControlButton(m_ui->vcm());
	m_otherCMCB->getControlBtn()->button()->setVisible(false);
	m_otherCMCB->getControlBtn()->setName("Other");
	m_ui->vcm()->addEnd(m_otherCMCB);
}

void ADCInstrumentController::deinit()
{
	for(auto c : qAsConst(m_components)) {
		c->onDeinit();
	}
}

void ADCInstrumentController::onStart()
{
	for(auto c : qAsConst(m_components)) {
		if(c->enabled()) {
			c->onStart();
		}
	}
	m_started = true;
}



void ADCInstrumentController::onStop()
{
	m_started = false;
	//dynamic_cast<GRTimeSinkComponent*>(m_dataProvider)->onStop();
	for(int idx = m_components.size() - 1 ; idx >= 0;idx--) {
		auto c = m_components[idx];
		c->onStop();
	}
}

void ADCInstrumentController::start()
{
	ResourceManager::open("adc",this);
	m_dataProvider->start();
}

void ADCInstrumentController::stop()
{
	m_dataProvider->stop();
	ResourceManager::close("adc");
}

void ADCInstrumentController::stopUpdates()
{
	qInfo(CAT_TIMEPLOT_PROXY) << "Stopped plotting";
	m_refreshTimerRunning = false;
	m_refillFuture.cancel();
	m_plotTimer->stop();
	m_ui->stopped();
}

void ADCInstrumentController::startUpdates()
{
	qInfo(CAT_TIMEPLOT_PROXY) << "Start plotting";
	updateFrameRate();
	m_refreshTimerRunning = true;
	update();
	m_plotTimer->start();
	m_ui->started();
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
//	m_dataProvider->setData(false);
	if(m_dataProvider->finished()) {
		Q_EMIT requestStop();
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
//		m_acqNodeComponentMap[grtbn] = (c);
		//addComponent(c);

		m_dataProvider = c;
		c->init();

		connect(m_timePlotSettingsComponent, &TimePlotManagerSettings::bufferSizeChanged, c,
			&GRTimeSinkComponent::setBufferSize);

		connect(c, &GRTimeSinkComponent::requestSingleShot, this, &ADCInstrumentController::setSingleShot);
		connect(c, &GRTimeSinkComponent::requestBufferSize, m_timePlotSettingsComponent, &TimePlotManagerSettings::setBufferSize);

		connect(m_timePlotSettingsComponent, &TimePlotManagerSettings::plotSizeChanged, c,
			&GRTimeSinkComponent::setPlotSize);
		connect(m_timePlotSettingsComponent, &TimePlotManagerSettings::sampleRateChanged, c,
			&GRTimeSinkComponent::setSampleRate);
		connect(m_timePlotSettingsComponent, &TimePlotManagerSettings::rollingModeChanged, c,
			&GRTimeSinkComponent::setRollingMode);

		connect(m_ui->m_singleBtn, &QAbstractButton::toggled, this, [=](bool b){
			setSingleShot(b);
			if(b && !m_started){
				Q_EMIT requestStart();
			}
		});
		connect(m_ui, &ADCInstrument::requestStart, this, &ADCInstrumentController::requestStart);
		connect(this, &ADCInstrumentController::requestStart, this, &ADCInstrumentController::start);
		connect(m_ui, &ADCInstrument::requestStop, this, &ADCInstrumentController::requestStop);
		connect(this, &ADCInstrumentController::requestStop, this, &ADCInstrumentController::stop);

		connect(m_ui->m_sync, &QAbstractButton::toggled, this, [=](bool b){
			c->setSyncMode(b);
		});

		connect(c, SIGNAL(arm()), this, SLOT(onStart()));
		connect(c, SIGNAL(disarm()), this, SLOT(onStop()));

		connect(c, SIGNAL(ready()), this, SLOT(startUpdates()));
		connect(c, SIGNAL(finish()), this, SLOT(stopUpdates()));
		connect(c, SIGNAL(refresh()), this, SLOT(update()));
	}

	if(dynamic_cast<GRIIODeviceSourceNode *>(node) != nullptr) {
		GRIIODeviceSourceNode *griiodsn = dynamic_cast<GRIIODeviceSourceNode *>(node);
		GRDeviceComponent *d = new GRDeviceComponent(griiodsn);
		addComponent(d);
		m_ui->addDevice(d->ctrl(), d);

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
			dynamic_cast<GRTimeSinkComponent *>(m_dataProvider);
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
			cw = m_ui->vcm();
		}
		m_acqNodeComponentMap[griiofcn] = c;

		/*** End of mess ***/

		m_ui->addChannel(c->ctrl(), c, cw);

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
		m_ui->addChannel(c->ctrl(), c, cw);

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

ADCInstrument *ADCInstrumentController::ui() const
{
	return m_ui;
}


