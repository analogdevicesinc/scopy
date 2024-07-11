#include "adcfftinstrumentcontroller.h"
#include "adcinstrument.h"
#include "grdevicecomponent.h"
#include "grtimechannelcomponent.h"
#include "importchannelcomponent.h"
#include "grtimesinkcomponent.h"
#include "freq/fftplotmanager.h"
#include "freq/fftplotmanagersettings.h"

using namespace scopy;
using namespace adc;

ADCFFTInstrumentController::ADCFFTInstrumentController(ToolMenuEntry *tme, QString name, AcqTreeNode *tree, QObject *parent) : ADCInstrumentController(tme,name, tree, parent)
{

}

ADCFFTInstrumentController::~ADCFFTInstrumentController()
{

}

void ADCFFTInstrumentController::init()
{
	ToolTemplate *toolLayout = m_ui->getToolTemplate();

	m_plotComponentManager = new FFTPlotManager(m_name + "_time", m_ui);
	addComponent(m_plotComponentManager);
	m_fftPlotSettingsComponent = new FFTPlotManagerSettings(dynamic_cast<FFTPlotManager*>(m_plotComponentManager));
	addComponent(m_fftPlotSettingsComponent);

	uint32_t tmp;
	tmp = m_plotComponentManager->addPlot("Acceleration 1");
	m_fftPlotSettingsComponent->addPlot(dynamic_cast<FFTPlotComponent*>(m_plotComponentManager->plot(tmp)));

	// m_cursorComponent = new CursorComponent(m_plotComponentManager, m_tool->getToolTemplate(), this);
	// addComponent(m_cursorComponent);

	m_measureComponent = new MeasureComponent(m_ui->getToolTemplate(), m_plotComponentManager, this);
	// m_measureComponent->addPlotComponent(m_plotComponentManager);

	addComponent(m_measureComponent);

	plotStack = new MapStackedWidget(m_ui);
	toolLayout->addWidgetToCentralContainerHelper(plotStack);

	plotStack->add("time", m_plotComponentManager);
	toolLayout->rightStack()->add(m_ui->settingsMenuId, m_fftPlotSettingsComponent);

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

void ADCFFTInstrumentController::addChannel(AcqTreeNode *node)
{
	qInfo() << node->name();

	if(dynamic_cast<GRTopBlockNode *>(node) != nullptr) {
		GRTopBlockNode *grtbn = dynamic_cast<GRTopBlockNode *>(node);
		GRTimeSinkComponent *c = new GRTimeSinkComponent(m_name + "_time", grtbn, this);
		//m_acqNodeComponentMap[grtbn] = (c);
		//addComponent(c);

		m_dataProvider = c;
		c->init();

		connect(m_fftPlotSettingsComponent, &FFTPlotManagerSettings::bufferSizeChanged, c,
			&GRTimeSinkComponent::setBufferSize);

		connect(c, &GRTimeSinkComponent::requestSingleShot, this, &ADCFFTInstrumentController::setSingleShot);
		connect(c, &GRTimeSinkComponent::requestBufferSize, m_fftPlotSettingsComponent, &FFTPlotManagerSettings::setBufferSize);


		connect(m_fftPlotSettingsComponent, &FFTPlotManagerSettings::sampleRateChanged, c,
			&GRTimeSinkComponent::setSampleRate);

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

	if(dynamic_cast<GRIIODeviceSourceNode *>(node) != nullptr) {
		GRIIODeviceSourceNode *griiodsn = dynamic_cast<GRIIODeviceSourceNode *>(node);
		GRDeviceComponent *d = new GRDeviceComponent(griiodsn);
		addComponent(d);
		m_ui->addDevice(d->ctrl(), d);

		m_acqNodeComponentMap[griiodsn] = (d);
		m_fftPlotSettingsComponent->addSampleRateProvider(d);
		addComponent(d);

		connect(m_fftPlotSettingsComponent, &FFTPlotManagerSettings::bufferSizeChanged, d,
			&GRDeviceComponent::setBufferSize);
	}

#if 0
	if(dynamic_cast<GRIIOFloatChannelNode *>(node) != nullptr) {
		int idx = chIdP->next();
		GRIIOFloatChannelNode *griiofcn = dynamic_cast<GRIIOFloatChannelNode *>(node);
		GRTimeSinkComponent *grtsc =
			dynamic_cast<GRTimeSinkComponent *>(m_dataProvider);
		GRTimeChannelComponent *c =
			new GRTimeChannelComponent(griiofcn, dynamic_cast<TimePlotComponent*>(m_plotComponentManager->plot(0)), grtsc, chIdP->pen(idx));
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

		addComponent(c);
		setupChannelMeasurement(m_plotComponentManager, c);
	}
#endif

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

		m_fftPlotSettingsComponent->addChannel(c); // SingleY/etc

		addComponent(c);
		setupChannelMeasurement(m_plotComponentManager, c);
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
