#ifndef TIMEPLOTPROXY_H
#define TIMEPLOTPROXY_H

#include "toolcomponent.h"
#include "scopy-adcplugin_export.h"
#include "plotcomponent.h"
#include "timeplotsettingscomponent.h"

namespace scopy {
namespace adc {
class ChannelIdProvider;

class SCOPY_ADCPLUGIN_EXPORT TimePlotProxy : public QObject, public PlotProxy, public AcqNodeChannelAware
{
	Q_OBJECT
public:
	TimePlotProxy(AcqTreeNode *tree, QObject *parent = nullptr);
	~TimePlotProxy();

	ChannelIdProvider *getChannelIdProvider();
	;

	void setInstrument(ADCTimeInstrument *tool);
	// PlotProxy interface
public:
	ToolComponent *getPlotAddon();
	ToolComponent *getPlotSettings();

	QList<ToolComponent *> getChannelAddons();
	QList<ToolComponent *> getComponents();

	QWidget *getInstrument() override;
	void setInstrument(QWidget *) override;

public Q_SLOTS:
	void init() override;
	void deinit() override;
	void onStart() override;
	void onStop() override;

	void addChannel(AcqTreeNode *c) override;
	void removeChannel(AcqTreeNode *c) override;

private:
	ADCTimeInstrument *m_tool;
	PlotComponent *m_plotComponent;
	TimePlotSettingsComponent *m_plotSettingsComponent;
	ChannelIdProvider *chIdP;

	QList<ToolComponent *> m_components;
	AcqTreeNode *m_tree;
	QMap<AcqTreeNode*, ToolComponent*> m_acqNodeComponentMap;
};
} // namespace adc
} // namespace scopy
#endif // TIMEPLOTPROXY_H
