#ifndef GRTIMEPLOTADDONSETTINGS_H
#define GRTIMEPLOTADDONSETTINGS_H

#include "grtimeplotaddon.h"
#include "scopy-gr-util_export.h"
#include "time_yautoscale.h"
#include "timechanneladdon.h"
#include "tooladdon.h"

#include <QLabel>

#include <gui/spinbox_a.hpp>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>

namespace scopy {
namespace grutil {
class TimeYControl;
class GRTimePlotAddon;
class GRTimeChannelAddon;
class SCOPY_GR_UTIL_EXPORT GRTimePlotAddonSettings : public QObject,
						     public ToolAddon,
						     public ChannelConfigAware,
						     public GRTopAddon
{
	Q_OBJECT
public:
	typedef enum
	{
		XMODE_SAMPLES,
		XMODE_TIME,
		XMODE_OVERRIDE
	} XMode;

	GRTimePlotAddonSettings(GRTimePlotAddon *p, QObject *parent = nullptr);
	~GRTimePlotAddonSettings();

	QString getName() override;
	QWidget *getWidget() override;

	uint32_t bufferSize() const;

	uint32_t plotSize() const;
	void setPlotSize(uint32_t newPlotSize);

	bool rollingMode() const;
	void setRollingMode(bool newRollingMode);

	bool showPlotTags() const;
	void setShowPlotTags(bool newShowPlotTags);

	double sampleRate() const;
	void setSampleRate(double newSampleRate);

public Q_SLOTS:
	void enable() override {}
	void disable() override {}
	void onStart() override {}
	void onStop() override {}
	void onInit() override;
	void onDeinit() override;
	void preFlowBuild() override;

	void onChannelAdded(ChannelAddon *t) override;
	void onChannelRemoved(ChannelAddon *t) override;

	void setBufferSize(uint32_t newBufferSize);
	void computeSampleRateAvailable();
	void showPlotLabels(bool);

Q_SIGNALS:
	void bufferSizeChanged(uint32_t);
	void plotSizeChanged(uint32_t);
	void rollingModeChanged(bool);
	void showPlotTagsChanged(bool);
	void sampleRateChanged(double);
	void singleYMode(bool);

private:
	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createXAxisMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent = nullptr);
	QWidget *createXYMenu(QWidget *parent = nullptr);
	QWidget *createFFTMenu(QWidget *parent = nullptr);

	double readSampleRate();

	GRTimePlotAddon *m_plot;
	QList<ChannelAddon *> channels;
	QString name;
	QWidget *widget;
	QPen m_pen;

	TimeYControl *m_yctrl;
	MenuOnOffSwitch *m_singleYModeSw;
	QPushButton *m_autoscaleBtn;
	TimeYAutoscale *autoscaler;

	ScaleSpinButton *m_bufferSizeSpin;
	ScaleSpinButton *m_plotSizeSpin;

	MenuComboWidget *xy_xaxis;
	MenuComboWidget *xy_yaxis;

	MenuOnOffSwitch *fft_complex_mode;
	MenuComboWidget *fft_i;
	MenuComboWidget *fft_q;

	PositionSpinButton *m_xmin;
	PositionSpinButton *m_xmax;
	PositionSpinButton *m_sampleRateSpin;
	MenuOnOffSwitch *m_rollingModeSw;
	MenuOnOffSwitch *m_syncBufferPlot;
	MenuOnOffSwitch *m_showTagsSw;
	MenuOnOffSwitch *m_showLabels;
	MenuCombo *m_xModeCb;

	bool m_sampleRateAvailable;

	uint32_t m_bufferSize;
	uint32_t m_plotSize;
	double m_sampleRate;
	bool m_rollingMode;
	bool m_showPlotTags;

	Q_PROPERTY(uint32_t bufferSize READ bufferSize WRITE setBufferSize NOTIFY bufferSizeChanged)
	Q_PROPERTY(uint32_t plotSize READ plotSize WRITE setPlotSize NOTIFY plotSizeChanged)
	Q_PROPERTY(bool rollingMode READ rollingMode WRITE setRollingMode NOTIFY rollingModeChanged)
	Q_PROPERTY(bool showPlotTags READ showPlotTags WRITE setShowPlotTags NOTIFY showPlotTagsChanged)

	Q_PROPERTY(double sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)
};
} // namespace grutil
} // namespace scopy

#endif // GRTIMEPLOTADDONSETTINGS_H
