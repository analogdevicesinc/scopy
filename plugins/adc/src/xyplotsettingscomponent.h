#ifndef XYPLOTSETTINGSCOMPONENT_H
#define XYPLOTSETTINGSCOMPONENT_H

#include "scopy-adcplugin_export.h"
#include "plotcomponent.h"
#include "toolcomponent.h"

#include <QLabel>

#include <gui/spinbox_a.hpp>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/plotautoscaler.h>
#include "channelcomponent.h"
#include "interfaces.h"

namespace scopy {
namespace adc {

using namespace scopy::gui;
#if 0
class SCOPY_ADCPLUGIN_EXPORT XyPlotSettingsComponent : public QWidget,
							 public ToolComponent
{
	Q_OBJECT
public:

	XyPlotSettingsComponent(PlotComponent *plot, QWidget *parent = nullptr);
	~XyPlotSettingsComponent();

	bool singleYMode() const;
	void setSingleYMode(bool newSingleYMode);

	uint32_t bufferSize() const;
	void setBufferSize(uint32_t newBufferSize);

	ChannelComponent *xChannel() const;
	void setXChannel(ChannelComponent *newXChannel);

public Q_SLOTS:
	void onStart() override;
	void onStop() override {}
	void onInit() override;
	void onDeinit() override {}
	void showPlotLabels(bool);

	void addChannel(ChannelComponent *c);
	void removeChannel(ChannelComponent *c);

Q_SIGNALS:
	void bufferSizeChanged(uint32_t);
	void singleYModeChanged(bool);
	void xChannelChanged(ChannelComponent*);

private:
	PlotWidget *m_plot;
	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createXAxisMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent = nullptr);

	QPen m_pen;
	MenuCombo *m_xChannelCb;
	ChannelComponent *m_xChannel;
	MenuPlotAxisRangeControl *m_xctrl;
	MenuPlotAxisRangeControl *m_yctrl;
	MenuOnOffSwitch *m_singleYModeSw;
	QPushButton *m_autoscaleBtn;
	PlotAutoscaler *y_autoscaler;
	PlotAutoscaler *x_autoscaler;

	ScaleSpinButton *m_bufferSizeSpin;
	MenuOnOffSwitch *m_showLabels;

	uint32_t m_bufferSize;
	bool m_syncMode;
	bool m_singleYMode;

	QList<ChannelComponent*> m_channels;
	QList<SampleRateProvider*> m_sampleRateProviders;
	// bool m_showPlotTags;

	Q_PROPERTY(bool singleYMode READ singleYMode WRITE setSingleYMode NOTIFY singleYModeChanged)
	Q_PROPERTY(uint32_t bufferSize READ bufferSize WRITE setBufferSize NOTIFY bufferSizeChanged FINAL)
	Q_PROPERTY(ChannelComponent* xChannel READ xChannel WRITE setXChannel NOTIFY xChannelChanged)

};
#endif
} // namespace adc
} // namespace scopy

#endif // XYPLOTSETTINGSCOMPONENT_H
