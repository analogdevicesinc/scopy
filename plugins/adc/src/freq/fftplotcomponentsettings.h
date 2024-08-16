#ifndef FFTPLOTCOMPONENTSETTINGS_H
#define FFTPLOTCOMPONENTSETTINGS_H

#include <QWidget>
#include <toolcomponent.h>
#include "menuspinbox.h"
#include "scopy-adc_export.h"
#include "channelcomponent.h"
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>

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

	QList<ChannelComponent *> m_channels;
	QPushButton *m_deletePlot;
	QPushButton *m_deletePlotHover;
	QPushButton *m_settingsPlotHover;

	bool m_autoscaleEnabled;
	bool m_running;

private:
	void toggleAutoScale();
	void updateYModeCombo();

	QMap<ChannelComponent*, QList<QMetaObject::Connection>> connections;
};
}}

#endif // FFTPLOTCOMPONENTSETTINGS_H
