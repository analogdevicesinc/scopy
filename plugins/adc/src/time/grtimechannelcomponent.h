#ifndef GRTIMECHANNELCOMPONENT_H
#define GRTIMECHANNELCOMPONENT_H

#include "grtimesinkcomponent.h"
#include "iioutil/iiounits.h"
#include "scopy-adc_export.h"
#include <gui/channelcomponent.h>
#include <gui/plotautoscaler.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menucontrolbutton.h>
#include "adcinterfaces.h"
#include <iio-widgets/iiowidget.h>
#include <gui/widgets/menuwidget.h>
#include "time/timeplotcomponent.h"

namespace scopy {
namespace adc {

using namespace scopy::gui;

class GRDeviceAddon;

class GRTimeChannelSigpath : public QObject, public GRChannel
{
	Q_OBJECT
public:
	GRTimeChannelSigpath(QString m_name, ChannelComponent *ch, GRIIOFloatChannelNode *node, QObject *parent)
		: QObject(parent)
	{
		m_ch = ch;
		m_node = node;
		m_grch = node->src();
		m_signalPath = new GRSignalPath(
			m_name + m_grch->getDeviceSrc()->deviceName() + m_grch->getChannelName(), this);
		m_signalPath->append(m_grch);
		m_scOff = new GRScaleOffsetProc(m_signalPath);
		m_signalPath->append(m_scOff);
		m_scOff->setOffset(0);
		m_scOff->setScale(1);
		m_signalPath->setEnabled(false);
		m_node->top()->src()->registerSignalPath(m_signalPath);
	}
	~GRTimeChannelSigpath() { m_node->top()->src()->unregisterSignalPath(m_signalPath); }

	void onNewData(const float *xData, const float *yData, size_t size, bool copy) override
	{
		m_ch->chData()->onNewData(xData, yData, size, copy);
	}

	GRSignalPath *sigpath() override { return m_signalPath; }

	ChannelComponent *m_ch;
	GRIIOFloatChannelNode *m_node;
	GRSignalPath *m_signalPath;
	GRScaleOffsetProc *m_scOff;
	GRIIOFloatChannelSrc *m_grch;
};

class SCOPY_ADC_EXPORT GRTimeChannelComponent : public ChannelComponent,
						public GRChannel,
						public MeasurementProvider,
						public SampleRateProvider,
						public ScaleProvider

{
	Q_OBJECT
public:
	GRTimeChannelComponent(GRIIOFloatChannelNode *node, TimePlotComponent *m_plot, GRTimeSinkComponent *grtsc,
			       QPen pen, QWidget *parent = nullptr);
	~GRTimeChannelComponent();

	MeasureManagerInterface *getMeasureManager() override;

	GRSignalPath *sigpath() override;
	QVBoxLayout *menuLayout();

	YMode ymode() const override;
	void setYMode(YMode newYmode) override;
	bool scaleAvailable() const override;
	double yMin() const override;
	double yMax() const override;
	bool yLock() const override;
	IIOUnit unit() const override;

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;

	void onNewData(const float *xData, const float *yData, size_t size, bool copy) override;

	bool sampleRateAvailable() override;
	double sampleRate() override;

	void setSamplingInfo(SamplingInfo p) override;

	void toggleAutoScale();
	void setYModeHelper(YMode mode);

	void addChannelToPlot() override;
	void removeChannelFromPlot() override;

Q_SIGNALS:
	void yModeChanged();

private:
	GRIIOFloatChannelNode *m_node;
	GRIIOFloatChannelSrc *m_src;
	GRTimeChannelSigpath *m_grtch;
	QVBoxLayout *m_layScroll;

	TimeMeasureManager *m_measureMgr;
	MenuPlotAxisRangeControl *m_yCtrl;
	PlotAutoscaler *m_autoscaler;
	MenuOnOffSwitch *m_autoscaleBtn;
	MenuCombo *m_ymodeCb;
	IIOWidget *m_scaleWidget;

	MenuPlotChannelCurveStyleControl *m_curvemenu;
	MenuSectionCollapseWidget *m_yaxisMenu;
	TimePlotComponentChannel *m_timePlotComponentChannel;

	QPushButton *m_snapBtn;

	bool m_yLock;
	bool m_scaleAvailable;
	bool m_autoscaleEnabled;
	bool m_running;

	IIOUnit m_unit;

	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createAttrMenu(QWidget *parent);
	QWidget *createYAxisMenu(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);
	QPushButton *createSnapshotButton(QWidget *parent);

	Q_PROPERTY(YMode ymode READ ymode WRITE setYMode NOTIFY yModeChanged);

	YMode m_ymode;
};

} // namespace adc
} // namespace scopy
#endif // GRTIMECHANNELCOMPONENT_H
