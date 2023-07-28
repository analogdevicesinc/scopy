#ifndef GRTIMECHANNELADDON_H
#define GRTIMECHANNELADDON_H

#include "tooladdon.h"
#include "grsignalpath.h"
#include "grtimeplotaddon.h"
#include "griiodevicesource.h"
#include "griiofloatchannelsrc.h"

#include <QLabel>
#include "scopy-gr-util_export.h"
#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotchannel.h>
#include "grscaleoffsetproc.h"
#include <gui/spinbox_a.hpp>


namespace scopy::grutil {
class GRDeviceAddon;
class SCOPY_GR_UTIL_EXPORT GRTimeChannelAddon : public QObject, public ToolAddon {
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
	GRSignalPath *signalPath() const;
	PlotChannel *plotCh() const;

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onStart() override;
	void onStop() override;
	void onAdd() override;
	void onRemove() override;
	void onChannelAdded(ToolAddon*) override;
	void onChannelRemoved(ToolAddon*) override;

	void toggleAutoScale();
	void autoscale();
	void setYMode(YMode mode);
private:
	QString m_channelName;
	GRDeviceAddon* m_dev;
	GRScaleOffsetProc* m_scOff;
	GRSignalPath *m_signalPath;
	GRIIOFloatChannelSrc *m_grch;
	GRTimePlotAddon* m_plotAddon;
	QPen m_pen;
	QTimer *m_autoScaleTimer;

	ScaleSpinButton *m_ymin;
	ScaleSpinButton *m_ymax;

	PlotChannel *m_plotCh;
	PlotAxis *m_plotAxis;
	PlotAxisHandle *m_plotAxisHandle;

	bool m_scaleAvailable;
	bool m_running;
	bool m_autoscaleEnabled;
	QString m_unit;
	QString name;
	QWidget *widget;
	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);
};
}
#endif // GRTIMECHANNELADDON_H
