#ifndef GRTIMECHANNELADDON_H
#define GRTIMECHANNELADDON_H

#include "tooladdon.h"
#include "grsignalpath.h"
#include "grtimeplotaddon.h"
#include "griiodevicesource.h"
#include "griiofloatchannelsrc.h"
#include "measurementcontroller.h"
#include "measure.h"

#include <QLabel>
#include "scopy-gr-util_export.h"
#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotchannel.h>
#include "grscaleoffsetproc.h"
#include <gui/spinbox_a.hpp>

#include <gui/widgets/menucombo.h>


namespace scopy::grutil {
class GRDeviceAddon;
class SCOPY_GR_UTIL_EXPORT GRTimeChannelAddon : public QObject, public ToolAddon, public GRTopAddon {
	Q_OBJECT
public:
	typedef enum {
		YMODE_COUNT,
		YMODE_FS,
		YMODE_SCALE
	} YMode;
	GRTimeChannelAddon(QString ch, GRDeviceAddon* dev, GRTimePlotAddon* plotAddon, QPen pen, QObject *parent = nullptr);
	~GRTimeChannelAddon();

	QString getName() override;
	QWidget* getWidget() override;

	void setDevice(GRDeviceAddon *d);
	GRDeviceAddon* getDevice();

	QPen pen() const;
	bool enabled() const;
	GRSignalPath *signalPath() const;
	PlotChannel *plotCh() const;

	bool sampleRateAvailable() const;
	GRIIOFloatChannelSrc *grch() const;

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;
	void preFlowBuild() override;

	void onNewData(const float* xData, const float* yData, int size);

	void onChannelAdded(ToolAddon*) override;
	void onChannelRemoved(ToolAddon*) override;

	void toggleAutoScale();
	void autoscale();
	void setYMode(YMode mode);

Q_SIGNALS:
	void enableMeasurement(QWidget*);
	void disableMeasurement(QWidget*);
	void enableStat(QWidget*);
	void disableStat(QWidget*);


private:
	QString m_channelName;
	GRDeviceAddon* m_dev;
	GRScaleOffsetProc* m_scOff;
	GRSignalPath *m_signalPath;
	GRIIOFloatChannelSrc *m_grch;
	GRTimePlotAddon* m_plotAddon;
	QPen m_pen;
	QTimer *m_autoScaleTimer;

	TimeChannelMeasurementController *m_measureController;
	TimeMeasureModel *m_measureModel;

	PositionSpinButton *m_ymin;
	PositionSpinButton *m_ymax;
	MenuCombo *m_ymodeCb;

	PlotChannel *m_plotCh;
	PlotAxis *m_plotAxis;
	PlotAxisHandle *m_plotAxisHandle;

	bool m_enabled;
	bool m_scaleAvailable;
	bool m_sampleRateAvailable;
	bool m_running;
	bool m_autoscaleEnabled;

	QString m_unit;
	QString name;
	QWidget *widget;
	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);

	void initMeasure();
	void createMeasurementMenu(QWidget *parent);
	QWidget *createMeasurementMenuSection(QString category, QWidget *parent);
};
}
#endif // GRTIMECHANNELADDON_H
