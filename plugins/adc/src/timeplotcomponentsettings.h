#ifndef TIMEPLOTCOMPONENTSETTINGS_H
#define TIMEPLOTCOMPONENTSETTINGS_H
#include <QWidget>
#include <timeplotcomponent.h>
#include <toolcomponent.h>
#include "scopy-adcplugin_export.h"


namespace scopy {
namespace adc {

class SCOPY_ADCPLUGIN_EXPORT TimePlotComponentSettings : public QWidget, public ToolComponent
{
	Q_OBJECT
public:
	TimePlotComponentSettings(TimePlotComponent *plt, QWidget *parent = nullptr);
	~TimePlotComponentSettings();

public Q_SLOTS:
	void addChannel(ChannelComponent *c);
	void removeChannel(ChannelComponent *c);
	void onInit() override;
	void onDeinit() override;
	void onStart() override;
	void onStop() override;
private:

	PlotAutoscaler *m_autoscaler;
	TimePlotComponent *m_plotComponent;
	MenuCombo *m_xAxisSrc;
	MenuOnOffSwitch *m_xAxisShow;
	MenuOnOffSwitch *m_autoscaleBtn;
	QList<ChannelComponent*> m_channels;

	bool m_autoscaleEnabled;
	bool m_running;
private:
	void toggleAutoScale();
};

}
}

#endif // TIMEPLOTCOMPONENTSETTINGS_H
