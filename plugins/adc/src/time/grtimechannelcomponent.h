#ifndef GRTIMECHANNELCOMPONENT_H
#define GRTIMECHANNELCOMPONENT_H

#include "scopy-adcplugin_export.h"
#include "channelcomponent.h"
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
						      public MeasurementProvider
{
	Q_OBJECT
public:
	typedef enum
	{
		YMODE_COUNT,
		YMODE_FS,
		YMODE_SCALE
	} YMode;
	GRTimeChannelComponent(GRIIOFloatChannelNode *node, TimePlotComponent *m_plot,QPen pen,
			   QWidget *parent = nullptr);
	~GRTimeChannelComponent();

	MenuControlButton *ctrl();
	MeasureManagerInterface *getMeasureManager() override;

public Q_SLOTS:

	void enable() override;
	void disable() override;
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;

	void onNewData(const float *xData, const float *yData, int size);

	void toggleAutoScale();

	void setYMode(YMode mode);
	void setSingleYMode(bool);
	// void setSampleRate(double v) override;
/*
Q_SIGNALS:
	void addNewSnapshot(SnapshotProvider::SnapshotRecipe) override;*/

private:
	GRIIOFloatChannelNode *m_node;
	GRIIOFloatChannelSrc *m_src;

	TimeMeasureManager *m_measureMgr;
	MenuControlButton *m_ctrl;
	MenuPlotAxisRangeControl *m_yCtrl;
	PlotAutoscaler *m_autoscaler;
	MenuCombo *m_ymodeCb;
	MenuOnOffSwitch *m_autoscaleBtn;

	//QPushButton *m_snapBtn;

	bool m_scaleAvailable;
	bool m_autoscaleEnabled;
	bool m_running;

	QString m_unit;
	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createAttrMenu(QWidget *parent);
	QWidget *createYAxisMenu(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);
	//QPushButton *createSnapshotButton(QWidget *parent);

	void createMenuControlButton(QWidget *parent = nullptr);
	void setupChannelMenuControlButtonHelper(MenuControlButton *btn);
};

} // namespace grutil
} // namespace scopy
#endif // GRTIMECHANNELCOMPONENT_H
