#ifndef TIMEPLOTCOMPONENTSETTINGS_H
#define TIMEPLOTCOMPONENTSETTINGS_H
#include <QWidget>
#include <timeplotcomponent.h>
#include <toolcomponent.h>
#include "scopy-adc_export.h"
#include "interfaces.h"
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT TimePlotComponentSettings : public QWidget, public ToolComponent
{
	Q_OBJECT
public:
	TimePlotComponentSettings(TimePlotComponent *plt, QWidget *parent = nullptr);
	~TimePlotComponentSettings();

	void showDeleteButtons(bool b);

public Q_SLOTS:
	void addChannel(ChannelComponent *c);
	void removeChannel(ChannelComponent *c);
	void onInit() override;
	void onDeinit() override;
	void onStart() override;
	void onStop() override;
	void updateYAxis();

Q_SIGNALS:
	void requestDeletePlot();

private:
	PlotAutoscaler *m_autoscaler;
	TimePlotComponent *m_plotComponent;
	MenuCombo *m_yModeCb;
	MenuCombo *m_xAxisSrc;
	MenuPlotAxisRangeControl *m_yCtrl;
	MenuPlotChannelCurveStyleControl *m_curve;
	MenuOnOffSwitch *m_xAxisShow;
	MenuOnOffSwitch *m_autoscaleBtn;
	QList<ChannelComponent *> m_channels;
	QList<ScaleProvider *> m_scaleProviders;
	QPushButton *m_deletePlot;
	QPushButton *m_deletePlotHover;

	bool m_autoscaleEnabled;
	bool m_running;




private:
	void toggleAutoScale();
	void updateYModeCombo();

};

} // namespace adc
} // namespace scopy

#endif // TIMEPLOTCOMPONENTSETTINGS_H
