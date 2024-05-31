#ifndef TIMEPLOTMANAGERSETTINGS_H
#define TIMEPLOTMANAGERSETTINGS_H

#include "scopy-adc_export.h"
#include <timeplotcomponent.h>
#include <toolcomponent.h>

#include <QLabel>

#include <gui/spinbox_a.hpp>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/plotautoscaler.h>
#include "channelcomponent.h"
#include "interfaces.h"

#include "timeplotmanager.h"

namespace scopy {
namespace adc {

using namespace scopy::gui;

class SCOPY_ADC_EXPORT TimePlotManagerSettings : public QWidget,
						       public ToolComponent
{
	Q_OBJECT
public:
	typedef enum
	{
		XMODE_SAMPLES,
		XMODE_TIME,
		XMODE_OVERRIDE
	} XMode;

	TimePlotManagerSettings(TimePlotManager *plot, QWidget *parent = nullptr);
	~TimePlotManagerSettings();

	uint32_t plotSize() const;
	void setPlotSize(uint32_t newPlotSize);

	bool rollingMode() const;
	void setRollingMode(bool newRollingMode);

	bool singleYMode() const;
	void setSingleYMode(bool newSingleYMode);

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

	void updateXAxis();

public Q_SLOTS:
	void onStart() override;
	void onStop() override {}
	void onInit() override;
	void onDeinit() override {}
	void showPlotLabels(bool);

	void addChannel(ChannelComponent *c);
	void removeChannel(ChannelComponent *c);

	void addSampleRateProvider(SampleRateProvider *s);
	void removeSampleRateProvider(SampleRateProvider *s);

	void addPlot(TimePlotComponent *plt);
	void removePlot(TimePlotComponent *p);
Q_SIGNALS:
	void plotSizeChanged(uint32_t);
	void bufferSizeChanged(uint32_t);

	void rollingModeChanged(bool);
	void sampleRateChanged(double);
	void singleYModeChanged(bool);
	void syncBufferPlotSizeChanged(bool);

private:
	TimePlotManager *m_plotManager;

	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createXAxisMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent = nullptr);
	double readSampleRate();
	void enableXModeTime();

	QPen m_pen;

	ScaleSpinButton *m_bufferSizeSpin;
	ScaleSpinButton *m_plotSizeSpin;

	PositionSpinButton *m_xmin;
	PositionSpinButton *m_xmax;
	PositionSpinButton *m_sampleRateSpin;
	PositionSpinButton *m_freqOffsetSpin;
	MenuOnOffSwitch *m_rollingModeSw;
	MenuOnOffSwitch *m_syncBufferPlot;
	MenuCombo *m_xModeCb;

	QPushButton *m_addPlotBtn;
	QVBoxLayout *m_plotContainerLayout;
	QMap<uint32_t, QWidget*> m_plotWidgetMap;

	bool m_sampleRateAvailable;
	uint32_t m_bufferSize;
	uint32_t m_plotSize;
	double m_sampleRate;
	bool m_rollingMode;
	bool m_syncBufferPlotSize;
	bool m_syncMode;

	QList<ChannelComponent*> m_channels;
	QList<SampleRateProvider*> m_sampleRateProviders;
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

#endif // TIMEPLOTMANAGERSETTINGS_H
