#ifndef TIMEPLOTSETTINGSCOMPONENT_H
#define TIMEPLOTSETTINGSCOMPONENT_H
/*
#include "scopy-adcplugin_export.h"
#include "toolcomponent.h"
#include "timechanneladdon.h"
#include "tooladdon.h"

#include <QLabel>

#include <gui/spinbox_a.hpp>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>

namespace scopy {
namespace adc {
class TimeYControl;
class GRTimePlotAddon;
class GRTimeChannelAddon;
class SCOPY_ADCPLUGIN_EXPORT TimePlotSettingsComponent
	: public QWidget
	, public ToolComponent
{
	Q_OBJECT
public:
	typedef enum
	{
		XMODE_SAMPLES,
		XMODE_TIME,
		XMODE_OVERRIDE
	} XMode;

	TimePlotSettingsComponent(GRTimePlotAddon *p, QObject *parent = nullptr);
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

public Q_SLOTS:
	void onStart() override {}
	void onStop() override {}
	void onInit() override;
	void onDeinit() override;
	void computeSampleRateAvailable();
	void showPlotLabels(bool);

Q_SIGNALS:
	void plotSizeChanged(uint32_t);
	void rollingModeChanged(bool);
	void sampleRateChanged(double);
	void singleYMode(bool);

private:
	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createXAxisMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent = nullptr);
	QWidget *createCurveMenu(PlotChannel *ch, QWidget *parent = nullptr);

	double readSampleRate();
	QString getComboNameFromChannelHelper(ChannelAddon *t);

	TimeYControl *m_yctrl;
	MenuOnOffSwitch *m_singleYModeSw;
	QPushButton *m_autoscaleBtn;
	TimeYAutoscale *autoscaler;

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
	double m_freqOffset;
	bool m_rollingMode;
	bool m_showPlotTags;

	Q_PROPERTY(uint32_t plotSize READ plotSize WRITE setPlotSize NOTIFY plotSizeChanged)
	Q_PROPERTY(bool rollingMode READ rollingMode WRITE setRollingMode NOTIFY rollingModeChanged)
	Q_PROPERTY(double sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)
};
} // namespace adc
} // namespace scopy
*/

#endif // TIMEPLOTSETTINGSCOMPONENT_H
