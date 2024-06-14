#ifndef GRTIMECHANNELCOMPONENT_H
#define GRTIMECHANNELCOMPONENT_H

#include "grtimesinkcomponent.h"
#include "menucollapsesection.h"
#include "scopy-adc_export.h"
#include "channelcomponent.h"
#include <gui/plotautoscaler.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menucontrolbutton.h>
#include "interfaces.h"
#include <iio-widgets/iiowidget.h>

namespace scopy {
namespace adc {

using namespace scopy::gui;

class GRDeviceAddon;

class GRTimeChannelSigpath : QObject
{
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
		m_signalPath->setEnabled(true); // or false
		m_node->top()->src()->registerSignalPath(m_signalPath);
	}
	~GRTimeChannelSigpath() {}

	void onNewData(const float *xData, const float *yData, size_t size, bool copy)
	{
		m_ch->chData()->onNewData(xData, yData, size, copy);
	}

	ChannelComponent *m_ch;
	GRIIOFloatChannelNode *m_node;
	GRSignalPath *m_signalPath;
	GRScaleOffsetProc *m_scOff;
	GRIIOFloatChannelSrc *m_grch;
};

class SCOPY_ADC_EXPORT GRTimeChannelComponent
	: public ChannelComponent
	, public GRChannel
	, public MeasurementProvider
	, public SampleRateProvider
	, public ScaleProvider

{
	Q_OBJECT
public:
	GRTimeChannelComponent(GRIIOFloatChannelNode *node, TimePlotComponent *m_plot, GRTimeSinkComponent *grtsc,
			       QPen pen, QWidget *parent = nullptr);
	~GRTimeChannelComponent();

	MenuControlButton *ctrl();
	MeasureManagerInterface *getMeasureManager() override;

	GRSignalPath *sigpath() override;
	void insertPlotComboWidget(QWidget *w);
	QVBoxLayout* menuLayout();

	YMode ymode() const override;
	void setYMode(YMode newYmode) override;
	bool scaleAvailable() const override;

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

	void toggleAutoScale();
	void setYModeHelper(YMode mode);

	/*
	Q_SIGNALS:
		void addNewSnapshot(SnapshotProvider::SnapshotRecipe) override;*/

signals:
	void yModeChanged();

private:
	GRIIOFloatChannelNode *m_node;
	GRIIOFloatChannelSrc *m_src;
	GRTimeChannelSigpath *m_grtch;
	QVBoxLayout *m_layScroll;

	TimeMeasureManager *m_measureMgr;
	MenuControlButton *m_ctrl;
	MenuOnOffSwitch *m_yAxisCtrl;
	MenuPlotAxisRangeControl *m_yCtrl;
	PlotAutoscaler *m_autoscaler;
	MenuCombo *m_ymodeCb;
	MenuOnOffSwitch *m_autoscaleBtn;
	IIOWidget* m_scaleWidget;

	MenuCollapseSection *m_curveSection;

	// QPushButton *m_snapBtn;

	bool m_scaleAvailable;
	bool m_autoscaleEnabled;
	bool m_running;

	QString m_unit;
	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createAttrMenu(QWidget *parent);
	QWidget *createYAxisMenu(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);
	// QPushButton *createSnapshotButton(QWidget *parent);

	void createMenuControlButton(QWidget *parent = nullptr);
	void setupChannelMenuControlButtonHelper(MenuControlButton *btn);

	Q_PROPERTY(YMode ymode READ ymode WRITE setYMode NOTIFY yModeChanged);

	YMode m_ymode;
};

} // namespace adc
} // namespace scopy
#endif // GRTIMECHANNELCOMPONENT_H
