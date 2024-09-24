#ifndef FFTPLOTCOMPONENTSETTINGS_H
#define FFTPLOTCOMPONENTSETTINGS_H

#include <QWidget>
#include "scopy-adc_export.h"
#include "menuspinbox.h"
#include "plotautoscaler.h"
#include <toolcomponent.h>
#include "channelcomponent.h"
#include <widgets/menuplotaxisrangecontrol.h>
#include <widgets/menuplotchannelcurvestylecontrol.h>

namespace scopy {
namespace adc {

class FFTPlotComponent;
class SCOPY_ADC_EXPORT FFTPlotComponentSettings : public QWidget, public ToolComponent
{
	Q_OBJECT
public:
	FFTPlotComponentSettings(FFTPlotComponent *plt, QWidget *parent = nullptr);
	~FFTPlotComponentSettings();

	void showDeleteButtons(bool b);
	void onStart() override;
	void onStop() override;

public Q_SLOTS:
	void addChannel(ChannelComponent *c);
	void removeChannel(ChannelComponent *c);

Q_SIGNALS:
	void requestDeletePlot();
	void requestSettings();

private:
	FFTPlotComponent *m_plotComponent;
	MenuPlotAxisRangeControl *m_yCtrl;
	MenuPlotChannelCurveStyleControl *m_curve;
	MenuSpinbox *m_yPwrOffset;
	PlotAutoscaler *m_autoscaler;

	MenuCombo *m_windowCb;
	MenuOnOffSwitch *m_windowChkb;

	QList<ChannelComponent *> m_channels;
	QPushButton *m_deletePlot;
	QPushButton *m_deletePlotHover;
	QPushButton *m_settingsPlotHover;

	bool m_autoscaleEnabled;
	bool m_running;

private:
	void toggleAutoScale();
	void updateYModeCombo();

	QMap<ChannelComponent *, QList<QMetaObject::Connection>> connections;
};
} // namespace adc
} // namespace scopy

#endif // FFTPLOTCOMPONENTSETTINGS_H
