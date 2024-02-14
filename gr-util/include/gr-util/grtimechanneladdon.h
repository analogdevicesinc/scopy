#ifndef GRTIMECHANNELADDON_H
#define GRTIMECHANNELADDON_H

#include "griiofloatchannelsrc.h"
#include "grscaleoffsetproc.h"
#include "grsignalpath.h"
#include "grtimeplotaddon.h"
#include "measurementcontroller.h"
#include "scopy-gr-util_export.h"
#include "time_yautoscale.h"
#include "time_ycontrol.h"
#include "timechanneladdon.h"
#include "tooladdon.h"

#include <QLabel>

#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotchannel.h>
#include <gui/spinbox_a.hpp>
#include <gui/widgets/menucombo.h>

namespace scopy {
class MenuOnOffSwitch;

namespace grutil {
class GRDeviceAddon;
class SCOPY_GR_UTIL_EXPORT GRTimeChannelAddon : public TimeChannelAddon, public GRTopAddon
{
	Q_OBJECT
public:
	typedef enum
	{
		YMODE_COUNT,
		YMODE_FS,
		YMODE_SCALE
	} YMode;
	GRTimeChannelAddon(QString ch, GRDeviceAddon *dev, GRTimePlotAddon *plotAddon, QPen pen,
			   QObject *parent = nullptr);
	~GRTimeChannelAddon();

	void setDevice(GRDeviceAddon *d);
	GRDeviceAddon *getDevice();

	GRSignalPath *signalPath() const;
	GRIIOFloatChannelSrc *grch() const;
	bool sampleRateAvailable() override;
	double sampleRate() override;
	MeasureManagerInterface *getMeasureManager() override;

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;
	void preFlowBuild() override;

	void onNewData(const float *xData, const float *yData, int size);

	void toggleAutoScale();
	void setYMode(YMode mode);
	void setSingleYMode(bool) override;

Q_SIGNALS:
	void addNewSnapshot(SnapshotProvider::SnapshotRecipe) override;

private:
	GRDeviceAddon *m_dev;
	GRScaleOffsetProc *m_scOff;
	GRSignalPath *m_signalPath;
	GRIIOFloatChannelSrc *m_grch;
	TimeMeasureManager *m_measureMgr;
	TimeYControl *m_yCtrl;
	TimeYAutoscale *m_autoscale;
	MenuCombo *m_ymodeCb;
	MenuOnOffSwitch *m_autoscaleBtn;
	QPushButton *m_snapBtn;

	bool m_scaleAvailable;
	bool m_sampleRateAvailable;
	bool m_autoscaleEnabled;
	bool m_running;

	QString m_unit;
	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createAttrMenu(QWidget *parent);
	QWidget *createYAxisMenu(QWidget *parent);
	QPushButton *createSnapshotButton(QWidget *parent);
};

class SCOPY_GR_UTIL_EXPORT ImportChannelAddon : public TimeChannelAddon
{
	Q_OBJECT
public:
	ImportChannelAddon(QString name, PlotAddon *plotAddon, QPen pen, QObject *parent = nullptr);
	virtual ~ImportChannelAddon();
	void setData(std::vector<float> x, std::vector<float> y);

	MeasureManagerInterface *getMeasureManager() override;
	void setSingleYMode(bool) override;
	bool sampleRateAvailable() override;
	double sampleRate() override;

	void onDeinit() override;
Q_SIGNALS:
	void addNewSnapshot(SnapshotProvider::SnapshotRecipe) override;

private:
	TimeMeasureManager *m_measureMgr;
	TimeYControl *m_yCtrl;
	TimeYAutoscale *m_autoscale;

	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent);
	QPushButton *createForgetButton(QWidget *parent);
};
} // namespace grutil
} // namespace scopy
#endif // GRTIMECHANNELADDON_H
