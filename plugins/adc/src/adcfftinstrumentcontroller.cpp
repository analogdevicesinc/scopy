#include "adcfftinstrumentcontroller.h"
#include "adcinstrument.h"
#include "grdevicecomponent.h"
#include "importchannelcomponent.h"
#include "freq/grfftsinkcomponent.h"
#include "freq/grfftchannelcomponent.h"
#include "freq/fftplotmanager.h"
#include "freq/fftplotmanagersettings.h"
#include "interfaces.h"
#include "importchannelcomponent.h"

using namespace scopy;
using namespace adc;

ADCFFTInstrumentController::ADCFFTInstrumentController(ToolMenuEntry *tme, QString name, AcqTreeNode *tree, QObject *parent) : ADCInstrumentController(tme,name, tree, parent)
{
	m_defaultComplexCh = nullptr;
	m_defaultRealCh = nullptr;
}

ADCFFTInstrumentController::~ADCFFTInstrumentController()
{

}

void ADCFFTInstrumentController::init()
{
	ToolTemplate *toolLayout = m_ui->getToolTemplate();

	m_plotComponentManager = new FFTPlotManager(m_name + "_fft", m_ui);
	addComponent(m_plotComponentManager);

	CursorSettings *m_cursorSettings = new CursorSettings();
	HoverWidget *hoverSettings = new HoverWidget(m_cursorSettings, m_ui->m_cursor, m_ui);
	hoverSettings->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	hoverSettings->setContentPos(HoverPosition::HP_TOPLEFT);
	hoverSettings->setAnchorOffset(QPoint(0, -10));

	connect(m_ui->m_cursor->button(), &QAbstractButton::toggled, hoverSettings, &HoverWidget::setVisible);

	connect(m_plotComponentManager, &PlotManager::plotAdded, this ,[=](uint32_t uuid) {
		auto cursorController = m_plotComponentManager->plot(uuid)->cursor();
		cursorController->connectSignals(m_cursorSettings);
		connect(m_ui->m_cursor, &QAbstractButton::toggled, cursorController, &CursorController::setVisible);
	});

	m_fftPlotSettingsComponent = new FFTPlotManagerSettings(dynamic_cast<FFTPlotManager*>(m_plotComponentManager));
	addComponent(m_fftPlotSettingsComponent);

	uint32_t tmp;
	tmp = m_plotComponentManager->addPlot("FFT");
	m_fftPlotSettingsComponent->addPlot(dynamic_cast<FFTPlotComponent*>(m_plotComponentManager->plot(tmp)));

	m_measureComponent = new MeasureComponent(m_ui->getToolTemplate(), m_plotComponentManager, this);
	// m_measureComponent->addPlotComponent(m_plotComponentManager);

	addComponent(m_measureComponent);

	plotStack = new MapStackedWidget(m_ui);
	toolLayout->addWidgetToCentralContainerHelper(plotStack);

	plotStack->add("fft", m_plotComponentManager);
	toolLayout->rightStack()->add(m_ui->settingsMenuId, m_fftPlotSettingsComponent);
	connect(m_fftPlotSettingsComponent, &FFTPlotManagerSettings::requestOpenMenu, [=]() {
		toolLayout->requestMenu(m_ui->settingsMenuId);
		m_ui->m_settingsBtn->setChecked(true);
	});

	connect(m_ui->m_printBtn, &QPushButton::clicked, this, [=, this]() {
		QList<PlotWidget *> plotList;

		for(PlotComponent* pp : m_plotComponentManager->plots()) {
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

	m_ui->m_settingsBtn->animateClick();
	m_ui->sync()->setVisible(false);
}

void ADCFFTInstrumentController::createIIODevice(AcqTreeNode *node)
{
	GRIIODeviceSourceNode *griiodsn = dynamic_cast<GRIIODeviceSourceNode *>(node);
	GRDeviceComponent *d = new GRDeviceComponent(griiodsn);
	addComponent(d);
	m_ui->addDevice(d->ctrl(), d);

	m_acqNodeComponentMap[griiodsn] = (d);
	m_fftPlotSettingsComponent->addSampleRateProvider(d);
	addComponent(d);

	connect(m_fftPlotSettingsComponent, &FFTPlotManagerSettings::samplingInfoChanged, this, [=](SamplingInfo p){
		d->setBufferSize(p.bufferSize);
	});
}

void ADCFFTInstrumentController::createIIOFloatChannel(AcqTreeNode *node)
{
	int idx = chIdP->next();
	GRIIOFloatChannelNode *griiofcn = dynamic_cast<GRIIOFloatChannelNode *>(node);
	GRFFTSinkComponent *grtsc =
		dynamic_cast<GRFFTSinkComponent *>(m_dataProvider);
	GRFFTChannelComponent *c =
		new GRFFTChannelComponent(griiofcn, dynamic_cast<FFTPlotComponent*>(m_plotComponentManager->plot(0)), grtsc, chIdP->pen(idx));
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
	m_fftPlotSettingsComponent->addChannel(c); // SingleY/etc
	connect(m_fftPlotSettingsComponent, &FFTPlotManagerSettings::samplingInfoChanged, c, &ChannelComponent::setSamplingInfo);

	addComponent(c);
	setupChannelMeasurement(m_plotComponentManager, c);

	if(m_defaultRealCh == nullptr) {
		m_defaultRealCh = c;
		m_plotComponentManager->selectChannel(c);
	}
}

void ADCFFTInstrumentController::createIIOComplexChannel(AcqTreeNode *node_I, AcqTreeNode *node_Q) {
	int idx = chIdP->next();
	GRIIOFloatChannelNode *griiofcn_I = dynamic_cast<GRIIOFloatChannelNode *>(node_I);
	GRIIOFloatChannelNode *griiofcn_Q = dynamic_cast<GRIIOFloatChannelNode *>(node_Q);
	GRFFTSinkComponent *grtsc =
		dynamic_cast<GRFFTSinkComponent *>(m_dataProvider);
	GRFFTChannelComponent *c =
		new GRFFTChannelComponent(griiofcn_I, griiofcn_Q, dynamic_cast<FFTPlotComponent*>(m_plotComponentManager->plot(0)), grtsc, chIdP->pen(idx));
	Q_ASSERT(grtsc);

	m_plotComponentManager->addChannel(c);
	QWidget *ww = m_plotComponentManager->plotCombo(c);
	c->menu()->add(ww, "plot", gui::MenuWidget::MA_BOTTOMFIRST);

	/*** This is a bit of a mess because CollapsableMenuControlButton is not a MenuControlButton ***/

	CompositeWidget *cw = nullptr;
	GRIIODeviceSourceNode *w = dynamic_cast<GRIIODeviceSourceNode *>(griiofcn_I->treeParent());
	GRDeviceComponent *dc = dynamic_cast<GRDeviceComponent *>(m_acqNodeComponentMap[w]);
	if(w) {
		cw = dc->ctrl();
	}
	if(!cw) {
		cw = m_ui->vcm();
	}
	/*** End of mess ***/

	m_ui->addChannel(c->ctrl(), c, cw);

	connect(c->ctrl(), &QAbstractButton::clicked, this,
		[=]() { m_plotComponentManager->selectChannel(c); });

	grtsc->addChannel(c);			    // For matching Sink To Channels
	dc->addChannel(c);			    // used for sample rate computation
	m_fftPlotSettingsComponent->addChannel(c); // SingleY/etc
	connect(m_fftPlotSettingsComponent, &FFTPlotManagerSettings::samplingInfoChanged, c, &ChannelComponent::setSamplingInfo);

	addComponent(c);
	setupChannelMeasurement(m_plotComponentManager, c);

	if(m_defaultComplexCh == nullptr) {
		m_defaultComplexCh = c;
	}
}

void ADCFFTInstrumentController::createFFTSink(AcqTreeNode *node)
{
	GRTopBlockNode *grtbn = dynamic_cast<GRTopBlockNode *>(node);
	GRFFTSinkComponent *c = new GRFFTSinkComponent(m_name + "_fft", grtbn, this);

	m_dataProvider = c;
	c->init();

	connect(m_fftPlotSettingsComponent, &FFTPlotManagerSettings::samplingInfoChanged, this, [=](SamplingInfo p){
		if(m_started) {
			stop();
			c->setSamplingInfo(p);
			start();
		} else {
			c->setSamplingInfo(p);
		}
	});

	connect(c, &GRFFTSinkComponent::requestSingleShot, this, &ADCFFTInstrumentController::setSingleShot);
	connect(c, &GRFFTSinkComponent::requestBufferSize, m_fftPlotSettingsComponent, &FFTPlotManagerSettings::setBufferSize);

	connect(m_ui->m_complex, &QAbstractButton::toggled, m_fftPlotSettingsComponent, &FFTPlotManagerSettings::setComplexMode);
	connect(m_ui->m_complex, &QAbstractButton::toggled, this, [=](){
		if(m_ui->m_complex->isChecked()) {
			m_plotComponentManager->selectChannel(m_defaultComplexCh);
		} else {
			m_plotComponentManager->selectChannel(m_defaultRealCh);
		}
	});

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
}

void ADCFFTInstrumentController::createImportFloatChannel(AcqTreeNode *node)
{
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

	m_fftPlotSettingsComponent->addChannel(c); // SingleY/etc

	addComponent(c);
	setupChannelMeasurement(m_plotComponentManager, c);
}

bool ADCFFTInstrumentController::getComplexChannelPair(AcqTreeNode *node, AcqTreeNode **node_i, AcqTreeNode **node_q)
{
	if(m_complexChannels.contains(node) && m_complexChannels.count() % 2 == 1) {
		// pending
		*node_i = node;
		*node_q = nullptr;
		return false;
	}

	if(m_complexChannels.contains(node) && m_complexChannels.count() % 2 == 0) {
		*node_i = nullptr;
		*node_q = nullptr;
		return false;
	}

	m_complexChannels.append(node);
	auto cnt = m_complexChannels.count();
	if(cnt == 1) {
		*node_i = node;
		*node_q = nullptr;
		return false;
	}

	*node_i = m_complexChannels[cnt-2];
	*node_q = m_complexChannels[cnt-1];
	return true;
}



void ADCFFTInstrumentController::addChannel(AcqTreeNode *node)
{
	qInfo() << node->name();

	if(dynamic_cast<GRTopBlockNode *>(node) != nullptr) {
		createFFTSink(node);
	}

	if(dynamic_cast<GRIIODeviceSourceNode *>(node) != nullptr) {
		createIIODevice(node);
	}

	if(dynamic_cast<GRIIOFloatChannelNode *>(node) != nullptr) {
		createIIOFloatChannel(node);
		AcqTreeNode *node_I, *node_Q;
		if(getComplexChannelPair(node, &node_I, &node_Q)) {
			createIIOComplexChannel(node_I, node_Q);
		}
	}

	if(dynamic_cast<ImportFloatChannelNode *>(node) != nullptr) {
		createImportFloatChannel(node);
	}

	m_plotComponentManager->replot();
}

void ADCFFTInstrumentController::removeChannel(AcqTreeNode *node)
{
	if(dynamic_cast<ImportFloatChannelNode *>(node) != nullptr) {
		ImportFloatChannelNode *ifcn = dynamic_cast<ImportFloatChannelNode *>(node);
		ImportChannelComponent *c = dynamic_cast<ImportChannelComponent *>(m_acqNodeComponentMap[ifcn]);

		m_otherCMCB->remove(c->ctrl());
		m_plotComponentManager->removeChannel(c);
		m_fftPlotSettingsComponent->removeChannel(c);
		removeComponent(c);
		delete c;
	}
	m_plotComponentManager->replot();
}
