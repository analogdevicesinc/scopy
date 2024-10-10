#include "adctimeinstrumentcontroller.h"
#include "adcinstrument.h"
#include "grdevicecomponent.h"
#include "grtimechannelcomponent.h"
#include "importchannelcomponent.h"
#include "grtimesinkcomponent.h"

using namespace scopy;
using namespace adc;

ADCTimeInstrumentController::ADCTimeInstrumentController(ToolMenuEntry *tme, QString name, AcqTreeNode *tree,
							 QObject *parent)
	: ADCInstrumentController(tme, name, tree, parent)
{
	m_defaultCh = nullptr;
}

ADCTimeInstrumentController::~ADCTimeInstrumentController() {}

void ADCTimeInstrumentController::init()
{
	ToolTemplate *toolLayout = m_ui->getToolTemplate();

	TimePlotManager *m_timePlotComponentManager = new TimePlotManager(m_name + "_time", m_ui);
	m_plotComponentManager = m_timePlotComponentManager;
	addComponent(m_plotComponentManager);

	CursorSettings *m_cursorSettings = new CursorSettings();
	HoverWidget *hoverSettings = new HoverWidget(m_cursorSettings, m_ui->m_cursor, m_ui);
	hoverSettings->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	hoverSettings->setContentPos(HoverPosition::HP_TOPLEFT);
	hoverSettings->setAnchorOffset(QPoint(0, -10));

	connect(m_ui->m_cursor->button(), &QAbstractButton::toggled, hoverSettings, &HoverWidget::setVisible);

	connect(m_plotComponentManager, &PlotManager::plotAdded, this, [=](uint32_t uuid) {
		auto cursorController = m_plotComponentManager->plot(uuid)->cursor();
		cursorController->connectSignals(m_cursorSettings);
		connect(m_ui->m_cursor, &QAbstractButton::toggled, cursorController, &CursorController::setVisible);
	});

	m_timePlotSettingsComponent = new TimePlotManagerSettings(m_timePlotComponentManager);
	addComponent(m_timePlotSettingsComponent);

	uint32_t tmp;
	tmp = m_plotComponentManager->addPlot("Time Plot");
	m_timePlotSettingsComponent->addPlot(m_timePlotComponentManager->plot(tmp));

	// m_cursorComponent = new CursorComponent(m_plotComponentManager, m_tool->getToolTemplate(), this);
	// addComponent(m_cursorComponent);

	m_measureComponent = new MeasureComponent(m_ui->getToolTemplate(), m_ui->getHoverMenuBtnGroup(),
						  m_plotComponentManager, this);
	// m_measureComponent->addPlotComponent(m_plotComponentManager);

	addComponent(m_measureComponent);

	plotStack = new MapStackedWidget(m_ui);
	toolLayout->addWidgetToCentralContainerHelper(plotStack);

	plotStack->add("time", m_plotComponentManager);
	toolLayout->rightStack()->add(m_ui->settingsMenuId, m_timePlotSettingsComponent);

	connect(m_timePlotSettingsComponent, &TimePlotManagerSettings::requestOpenMenu, [=]() {
		toolLayout->requestMenu(m_ui->settingsMenuId);
		m_ui->m_settingsBtn->setChecked(true);
	});

	connect(m_ui->m_printBtn, &QPushButton::clicked, this, [=, this]() {
		QList<PlotWidget *> plotList;

		for(PlotComponent *pp : m_plotComponentManager->plots()) {
			for(PlotWidget *plt : pp->plots()) {
				plotList.push_back(plt);
			}
		}
		m_ui->printPlotManager->printPlots(plotList, "ADC");
	});

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
	m_ui->m_complex->setVisible(false);

	m_ui->m_settingsBtn->animateClick();
	m_ui->sync()->setVisible(false);

	m_measureComponent->measureSettings()->getMarkerSection()->setVisible(false);
}

void ADCTimeInstrumentController::createTimeSink(AcqTreeNode *node)
{
	GRTopBlockNode *grtbn = dynamic_cast<GRTopBlockNode *>(node);
	GRTimeSinkComponent *c = new GRTimeSinkComponent(m_name + "_time", grtbn, this);
	//		m_acqNodeComponentMap[grtbn] = (c);
	// addComponent(c);

	m_dataProvider = c;
	c->init();

	connect(c, &GRTimeSinkComponent::requestSingleShot, this, &ADCTimeInstrumentController::setSingleShot);
	connect(c, &GRTimeSinkComponent::requestBufferSize, m_timePlotSettingsComponent,
		&TimePlotManagerSettings::setBufferSize);

	connect(m_timePlotSettingsComponent, &TimePlotManagerSettings::samplingInfoChanged, c,
		&GRTimeSinkComponent::setSamplingInfo);

	connect(m_ui->m_singleBtn, &QAbstractButton::toggled, this, [=](bool b) {
		setSingleShot(b);
		if(b && !m_started) {
			Q_EMIT requestStart();
		}
	});
	connect(m_ui, &ADCInstrument::requestStart, this, &ADCInstrumentController::requestStart);
	connect(this, &ADCInstrumentController::requestStart, this, &ADCInstrumentController::start);
	connect(m_ui, &ADCInstrument::requestStop, this, &ADCInstrumentController::requestStop);
	connect(this, &ADCInstrumentController::requestStop, this, &ADCInstrumentController::stop);

	connect(m_ui->m_sync, &QAbstractButton::toggled, this, [=](bool b) { c->setSyncMode(b); });

	connect(c, SIGNAL(arm()), this, SLOT(onStart()));
	connect(c, SIGNAL(disarm()), this, SLOT(onStop()));

	connect(c, SIGNAL(ready()), this, SLOT(startUpdates()));
	connect(c, SIGNAL(finish()), this, SLOT(stopUpdates()));
}

void ADCTimeInstrumentController::createIIODevice(AcqTreeNode *node)
{
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

void ADCTimeInstrumentController::createIIOFloatChannel(AcqTreeNode *node)
{
	int idx = chIdP->next();
	GRIIOFloatChannelNode *griiofcn = dynamic_cast<GRIIOFloatChannelNode *>(node);
	GRTimeSinkComponent *grtsc = dynamic_cast<GRTimeSinkComponent *>(m_dataProvider);
	GRTimeChannelComponent *c = new GRTimeChannelComponent(
		griiofcn, dynamic_cast<TimePlotComponent *>(m_plotComponentManager->plot(0)), grtsc, chIdP->pen(idx));
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

	connect(c->ctrl(), &QAbstractButton::clicked, this, [=]() { m_plotComponentManager->selectChannel(c); });

	grtsc->addChannel(c);			    // For matching Sink To Channels
	dc->addChannel(c);			    // used for sample rate computation
	m_timePlotSettingsComponent->addChannel(c); // SingleY/etc

	addComponent(c);
	setupChannelMeasurement(m_plotComponentManager, c);

	if(m_defaultCh == nullptr) {
		m_defaultCh = c;
		m_plotComponentManager->selectChannel(c);
	}
}

void ADCTimeInstrumentController::createImportFloatChannel(AcqTreeNode *node)
{
	int idx = chIdP->next();
	ImportFloatChannelNode *ifcn = dynamic_cast<ImportFloatChannelNode *>(node);
	ImportChannelComponent *c = new ImportChannelComponent(ifcn, chIdP->pen(idx));

	m_plotComponentManager->addChannel(c);
	c->menu()->add(m_plotComponentManager->plotCombo(c), "plot", gui::MenuWidget::MA_BOTTOMFIRST);

	CompositeWidget *cw = m_otherCMCB;
	m_acqNodeComponentMap[ifcn] = c;
	m_ui->addChannel(c->ctrl(), c, cw);

	connect(c->ctrl(), &QAbstractButton::clicked, this, [=]() { m_plotComponentManager->selectChannel(c); });

	c->ctrl()->animateClick();

	m_timePlotSettingsComponent->addChannel(c); // SingleY/etc

	addComponent(c);
	setupChannelMeasurement(m_plotComponentManager, c);
}

void ADCTimeInstrumentController::addChannel(AcqTreeNode *node)
{
	qInfo() << node->name();

	if(dynamic_cast<GRTopBlockNode *>(node) != nullptr) {
		createTimeSink(node);
	}

	if(dynamic_cast<GRIIODeviceSourceNode *>(node) != nullptr) {
		createIIODevice(node);
	}

	if(dynamic_cast<GRIIOFloatChannelNode *>(node) != nullptr) {
		createIIOFloatChannel(node);
	}

	if(dynamic_cast<ImportFloatChannelNode *>(node) != nullptr) {
		createImportFloatChannel(node);
	}
	m_plotComponentManager->replot();
}

void ADCTimeInstrumentController::removeChannel(AcqTreeNode *node)
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

void ADCTimeInstrumentController::setEnableAddRemovePlot(bool b)
{
	m_timePlotSettingsComponent->setEnableAddRemovePlot(b);
}
