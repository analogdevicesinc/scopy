#ifndef TIMEINSTRUMENTCOMPONENT_H
#define TIMEINSTRUMENTCOMPONENT_H
#include "scopy-adcplugin_export.h"
#include <plotcomponent.h>
#include <timeplotsettingscomponent.h>
#include <grtimesinkcomponent.h>
#include <adcinstrument.h>

namespace scopy {
namespace adc {
/*
class SCOPY_ADCPLUGIN_EXPORT TimeInstrumentComponent : public QObject, public MetaComponent, public AcqNodeChannelAware
{
	Q_OBJECT
public:
	TimeInstrumentComponent(ADCInstrument *adc, QObject* parent = nullptr);
	~TimeInstrumentComponent();

private:
	ADCInstrument *adc;
	PlotComponent *time;
	TimePlotSettingsComponent *m_timePlotSettingsComponent;
	GRTimeSinkComponent *m_timeSinkComponent;
	QList<ChannelComponent*> m_channels;

	ChannelIdProvider *chIdP;


	// ToolComponent interface
public:
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;
	void enable() override;
	void disable() override;

	// AcqNodeChannelAware interface
public:
	void addChannel(AcqTreeNode *c) override;
	void removeChannel(AcqTreeNode *c) override;

	// steps:

	// Or just  create a whole new tool ? - ToolTemplateV2 ?
	// figure out lifetime in ADCInstrument / ADCInstrumentController

	// init ui
	// init - acqtreeRoot
	// create enable/disable behavior
	// create sink->plotcontroller
	// create device components
	// create channel components
	// connect settings to sink and to (TimeChannel interface ? )

	// enable/disable cursors/measurement



};
*/
}
}
#endif // TIMEINSTRUMENTCOMPONENT_H
