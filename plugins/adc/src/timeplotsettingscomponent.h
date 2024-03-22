#ifndef TIMEPLOTSETTINGSCOMPONENT_H
#define TIMEPLOTSETTINGSCOMPONENT_H

#include "scopy-adcplugin_export.h"
#include "plotcomponent.h"
#include "toolcomponent.h"

#include <QLabel>

#include <gui/spinbox_a.hpp>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/plotautoscaler.h>

namespace scopy {
namespace adc {

using namespace scopy::gui;

class SCOPY_ADCPLUGIN_EXPORT TimePlotSettingsComponent : public QWidget, public ToolComponent
{
	Q_OBJECT
public:
	typedef enum
	{
		XMODE_SAMPLES,
		XMODE_TIME,
		XMODE_OVERRIDE
	} XMode;

	TimePlotSettingsComponent(PlotComponent *plot, QWidget *parent = nullptr);
	~TimePlotSettingsComponent();

	uint32_t plotSize() const;
	void setPlotSize(uint32_t newPlotSize);

	bool rollingMode() const;
	void setRollingMode(bool newRollingMode);

	bool showPlotTags() const;
	void setShowPlotTags(bool newShowPlotTags);

	double sampleRate() const;
	void setSampleRate(double newSampleRate);

	double freqOffset() const;
	void setFreqOffset(double newFreqOffset);

	bool syncBufferPlotSize() const;
	void setSyncBufferPlotSize(bool newSyncBufferPlotSize);

	uint32_t bufferSize() const;
	void setBufferSize(uint32_t newBufferSize);

public Q_SLOTS:
	void onStart() override {}
	void onStop() override {}
	void onInit() override {}
	void onDeinit() override {}
	void showPlotLabels(bool);

Q_SIGNALS:
	void plotSizeChanged(uint32_t);
	void bufferSizeChanged(uint32_t);

	void rollingModeChanged(bool);
	void sampleRateChanged(double);
	void singleYMode(bool);
	void syncBufferPlotSizeChanged(bool);

private:
	PlotWidget *m_plot;
	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createXAxisMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent = nullptr);

	QPen m_pen;
	MenuPlotAxisRangeControl *m_yctrl;
	MenuOnOffSwitch *m_singleYModeSw;
	QPushButton *m_autoscaleBtn;
	PlotAutoscaler *autoscaler;

	ScaleSpinButton *m_bufferSizeSpin;
	ScaleSpinButton *m_plotSizeSpin;

	PositionSpinButton *m_xmin;
	PositionSpinButton *m_xmax;
	PositionSpinButton *m_sampleRateSpin;
	PositionSpinButton *m_freqOffsetSpin;
	MenuOnOffSwitch *m_rollingModeSw;
	MenuOnOffSwitch *m_syncBufferPlot;
	MenuOnOffSwitch *m_showLabels;
	MenuCombo *m_xModeCb;

	bool m_sampleRateAvailable;

	uint32_t m_bufferSize;
	uint32_t m_plotSize;
	double m_sampleRate;
	bool m_rollingMode;
	bool m_syncBufferPlotSize;
	// bool m_showPlotTags;

	Q_PROPERTY(uint32_t plotSize READ plotSize WRITE setPlotSize NOTIFY plotSizeChanged)

	Q_PROPERTY(bool rollingMode READ rollingMode WRITE setRollingMode NOTIFY rollingModeChanged)
	Q_PROPERTY(double sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)
	Q_PROPERTY(bool syncBufferPlotSize READ syncBufferPlotSize WRITE setSyncBufferPlotSize NOTIFY
			   syncBufferPlotSizeChanged FINAL)
	Q_PROPERTY(uint32_t bufferSize READ bufferSize WRITE setBufferSize NOTIFY bufferSizeChanged FINAL)
};
} // namespace adc
} // namespace scopy

#endif // TIMEPLOTSETTINGSCOMPONENT_H
