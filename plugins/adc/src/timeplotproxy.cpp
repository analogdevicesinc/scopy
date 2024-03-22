#include "timeplotproxy.h"
#include "adcplugin.h"
#include "plotcomponent.h"
#include "adctimeinstrument.h"

using namespace scopy;
using namespace scopy::adc;

TimePlotProxy::TimePlotProxy(QObject *parent) : QObject(parent)
{
	chIdP = new ChannelIdProvider(this);


}

TimePlotProxy::~TimePlotProxy() {

}

ChannelIdProvider *TimePlotProxy::getChannelIdProvider() {
	return chIdP;
}

void TimePlotProxy::setInstrument(ADCTimeInstrument *t)
{
	m_tool = t;
}

ToolComponent *TimePlotProxy::getPlotAddon()
{
	return (ToolComponent*)m_plotComponent;
}

ToolComponent *TimePlotProxy::getPlotSettings()
{
	return (ToolComponent*)m_plotSettingsAddon;
}

QList<ToolComponent *> TimePlotProxy::getChannelAddons()
{
	/*QList<ToolComponent *> list;

	 for(ToolComponent* a : addons) {
		 if(dynamic_cast<TimeChannelComponent*>(a) )
			 list.append(a);
	 }*/
	return m_components;
}

QList<ToolComponent *> TimePlotProxy::getComponents()
{
	return m_components;
}

QWidget* TimePlotProxy::getInstrument()
{
	return (QWidget*)(m_tool);
}

void TimePlotProxy::setInstrument(QWidget* t)
{
	ADCTimeInstrument* ai = dynamic_cast<ADCTimeInstrument*>(t);
	Q_ASSERT(ai);
	m_tool = ai;
}

void TimePlotProxy::init()
{
	ToolTemplate* toolLayout = m_tool->getToolTemplate();
	m_plotComponent = new PlotComponent(m_tool);
	m_components.append(m_plotComponent);
	toolLayout->addWidgetToCentralContainerHelper(m_plotComponent);

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

void TimePlotProxy::addChannel(AcqTreeNode *c)
{
}

void TimePlotProxy::removeChannel(AcqTreeNode *c)
{

}
