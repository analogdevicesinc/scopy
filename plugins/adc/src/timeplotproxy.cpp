#include "timeplotproxy.h"
#include "adcplugin.h"
#include "plotcomponent.h"
#include "adctimeinstrument.h"
#include "grdevicecomponent.h"
#include "grtimechannelcomponent.h"

using namespace scopy;
using namespace scopy::adc;

TimePlotProxy::TimePlotProxy(AcqTreeNode * tree, QObject *parent)
	: QObject(parent)
{
	chIdP = new ChannelIdProvider(this);
	m_tree = tree;
}

TimePlotProxy::~TimePlotProxy() {}

ChannelIdProvider *TimePlotProxy::getChannelIdProvider() { return chIdP; }

void TimePlotProxy::setInstrument(ADCTimeInstrument *t) { m_tool = t; }

ToolComponent *TimePlotProxy::getPlotAddon() { return (ToolComponent *)m_plotComponent; }

ToolComponent *TimePlotProxy::getPlotSettings() { return (ToolComponent *)m_plotSettingsComponent; }

QList<ToolComponent *> TimePlotProxy::getChannelAddons()
{
	/*QList<ToolComponent *> list;

	 for(ToolComponent* a : addons) {
		 if(dynamic_cast<TimeChannelComponent*>(a) )
			 list.append(a);
	 }*/
	return m_components;
}

QList<ToolComponent *> TimePlotProxy::getComponents() { return m_components; }

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
	m_plotComponent = new PlotComponent(m_tool);
	addComponent(m_plotComponent);

	m_plotSettingsComponent = new TimePlotSettingsComponent(m_plotComponent);
	addComponent(m_plotSettingsComponent);

	toolLayout->addWidgetToCentralContainerHelper(m_plotComponent);
	toolLayout->rightStack()->add(m_tool->settingsMenuId, m_plotSettingsComponent);

	auto components = getComponents();
	for(auto c : components) {
		c->onInit();
	}
}

void TimePlotProxy::deinit()
{
	auto components = getComponents();
	for(auto c : components) {
		c->onDeinit();
	}
}

void TimePlotProxy::onStart()
{
	auto components = getComponents();

	for(auto c : components) {
		c->onStart();
	}
}

void TimePlotProxy::onStop()
{
	auto components = getComponents();

	for(auto c : components) {
		c->onStop();
	}
}

void TimePlotProxy::addChannel(AcqTreeNode *c) {
	qInfo()<<c->name();

	if(dynamic_cast<GRIIODeviceSourceNode*>(c) != nullptr) {
		GRIIODeviceSourceNode* griiodsn = dynamic_cast<GRIIODeviceSourceNode*>(c);
		GRDeviceComponent *d = new GRDeviceComponent(griiodsn);
		m_tool->addDevice(d->ctrl(), d);

		m_acqNodeComponentMap[griiodsn] = d;
	}

	if(dynamic_cast<GRIIOFloatChannelNode*>(c) != nullptr) {
		int idx = chIdP->next();
		GRIIOFloatChannelNode* griiofcn = dynamic_cast<GRIIOFloatChannelNode*>(c);
		GRTimeChannelComponent *c = new GRTimeChannelComponent(griiofcn, m_plotComponent, chIdP->pen(idx));

		CompositeWidget *cw = nullptr;
		GRIIODeviceSourceNode *w = dynamic_cast<GRIIODeviceSourceNode*>(griiofcn->treeParent());
		if(w) {
			cw = dynamic_cast<GRDeviceComponent*>(m_acqNodeComponentMap[w])->ctrl();
		}

		if(!cw) {
			cw = m_tool->vcm();
		}

		m_tool->addChannel(c->ctrl(), c, cw);
		m_acqNodeComponentMap[griiofcn] = c;
	}




	/*
	connect(s, &GRTimePlotAddonSettings::bufferSizeChanged, d, &GRDeviceAddon::updateBufferSize);
	recipe->addDeviceAddon(d);*/

	/*GRDeviceAddon *dev = dynamic_cast<GRDeviceAddon *>(d);
	if(!dev)
		continue;
	CollapsableMenuControlButton *devBtn = addDevice(dev, vcm);
	vcm->add(devBtn);
	*/


	/*GRTimeChannelAddon *t = new GRTimeChannelAddon(ch, d, p, chIdProvider->pen(idx), this);
	top->registerSignalPath(t->signalPath());*/
}

void TimePlotProxy::removeChannel(AcqTreeNode *c) {}


