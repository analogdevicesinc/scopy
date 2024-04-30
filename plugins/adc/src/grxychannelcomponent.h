#ifndef GRXYCHANNELCOMPONENT_H
#define GRXYCHANNELCOMPONENT_H

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
class SCOPY_ADCPLUGIN_EXPORT GRXyChannelComponent : public ChannelComponent,
						    public GRSignalPathProvider
{
	Q_OBJECT
public:
	typedef enum
	{
		YMODE_COUNT,
		YMODE_FS,
		YMODE_SCALE
	} YMode;
	GRXyChannelComponent(GRIIOFloatChannelNode *node, PlotComponent *plotComponent, QPen pen,
			       QWidget *parent = nullptr);
	~GRXyChannelComponent();

	GRSignalPath *signalPath() const override;
	GRIIOFloatChannelSrc *grch() const;
	MenuControlButton *ctrl();

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

	void setYMode(YMode mode);
	/*
	Q_SIGNALS:
		void addNewSnapshot(SnapshotProvider::SnapshotRecipe) override;*/

private:
	GRIIOFloatChannelNode *m_node;

	GRSignalPath *m_signalPath;
	GRScaleOffsetProc *m_scOff;
	GRIIOFloatChannelSrc *m_grch;

	MenuControlButton *m_ctrl;
	MenuCombo *m_ymodeCb;
	//QPushButton *m_snapBtn;

	bool m_scaleAvailable;
	bool m_running;
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

#endif // GRXYCHANNELCOMPONENT_H
