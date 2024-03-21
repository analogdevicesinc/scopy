#ifndef TIMEPLOTPROXY_H
#define TIMEPLOTPROXY_H

#include "toolcomponent.h"
#include "scopy-adcplugin_export.h"
#include "plotcomponent.h"

namespace scopy {
namespace adc {
class ChannelIdProvider;

class SCOPY_ADCPLUGIN_EXPORT TimePlotProxy : public QObject, public PlotProxy, public AcqNodeChannelAware
{
	Q_OBJECT
public:
	TimePlotProxy(QObject *parent = nullptr);
	~TimePlotProxy();

	ChannelIdProvider *getChannelIdProvider();;

	void setInstrument(ADCTimeInstrument *tool);
	// PlotProxy interface
public:

	ToolComponent *getPlotAddon();
	ToolComponent *getPlotSettings();

	QList<ToolComponent *> getChannelAddons();
	QList<ToolComponent *> getComponents();

	QWidget* getInstrument();
	void setInstrument(QWidget*);

public Q_SLOTS:
	void init();
	void deinit();
	void onStart();
	void onStop();

	void addChannel(AcqTreeNode *c);
	void removeChannel(AcqTreeNode *c);

private:
	ADCTimeInstrument *m_tool;
	PlotComponent *m_plotComponent;
	GRTimePlotAddonSettings *m_plotSettingsAddon;
	ChannelIdProvider *chIdP;
	QList<ToolComponent *> m_components;



	QString prefix;
};
}
}
#endif // TIMEPLOTPROXY_H
