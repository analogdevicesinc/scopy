#ifndef GRTIMECHANNELCOMPONENT_H
#define GRTIMECHANNELCOMPONENT_H

#include "scopy-adcplugin_export.h"
#include "channelcomponent.h"
#include "src/grdevicecomponent.h"
#include <gui/plotautoscaler.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menucontrolbutton.h>
#include "interfaces.h"
namespace scopy {
namespace adc {

using namespace scopy::gui;

class GRDeviceAddon;
class SCOPY_ADCPLUGIN_EXPORT GRTimeChannelComponent : public ChannelComponent,
						      public GRSignalPathProvider,
						      public SampleRateProvider,
						      public SingleYModeUser,
						      public MeasurementProvider,
						      public SampleRateUser
{
	Q_OBJECT
public:
	typedef enum
	{
		YMODE_COUNT,
		YMODE_FS,
		YMODE_SCALE
	} YMode;
	GRTimeChannelComponent(GRIIOFloatChannelNode *node, PlotComponent *plotComponent, QPen pen,
			   QWidget *parent = nullptr);
	~GRTimeChannelComponent();

	GRSignalPath *signalPath() const override;
	GRIIOFloatChannelSrc *grch() const;
	MenuControlButton *ctrl();

	bool sampleRateAvailable() override;
	double sampleRate() override;
	MeasureManagerInterface *getMeasureManager() override;
	void setSampleRate(double) override;

public Q_SLOTS:

	void enable() override;
	void disable() override;
	void enableChannel() override;
	void disableChannel() override;
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;

	void onNewData(const float *xData, const float *yData, int size);

	void toggleAutoScale();
	void setYMode(YMode mode);
	void setSingleYMode(bool) override;
/*
Q_SIGNALS:
	void addNewSnapshot(SnapshotProvider::SnapshotRecipe) override;*/

private:
	GRIIOFloatChannelNode *m_node;

	GRSignalPath *m_signalPath;
	GRScaleOffsetProc *m_scOff;

	GRIIOFloatChannelSrc *m_grch;
	TimeMeasureManager *m_measureMgr;
	MenuControlButton *m_ctrl;
	MenuPlotAxisRangeControl *m_yCtrl;
	PlotAutoscaler *m_autoscale;
	MenuCombo *m_ymodeCb;
	MenuOnOffSwitch *m_autoscaleBtn;
	//QPushButton *m_snapBtn;

	bool m_scaleAvailable;
	bool m_sampleRateAvailable;
	bool m_autoscaleEnabled;
	bool m_running;
	double m_plotSampleRate;

	QString m_unit;
	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createAttrMenu(QWidget *parent);
	QWidget *createYAxisMenu(QWidget *parent);
	//QPushButton *createSnapshotButton(QWidget *parent);

	void setupSignalPath();
	void createMenuControlButton(QWidget *parent = nullptr);
	void setupChannelMenuControlButtonHelper(MenuControlButton *btn);
};

} // namespace grutil
} // namespace scopy
#endif // GRTIMECHANNELCOMPONENT_H
