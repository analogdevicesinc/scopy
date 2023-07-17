#ifndef GRTIMECHANNELADDON_H
#define GRTIMECHANNELADDON_H

#include "tooladdon.h"
#include "grsignalpath.h"
#include "grtimeplotaddon.h"
#include "griiodevicesource.h"
#include <QLabel>
#include "scopy-gr-util_export.h"
#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotchannel.h>


namespace scopy::grutil {
class GRDeviceAddon;
class SCOPY_GR_UTIL_EXPORT GRTimeChannelAddon : public QObject, public ToolAddon {
	Q_OBJECT
public:
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

private:
	GRSignalPath *m_signalPath;
	GRIIOChannel *m_grch;
	GRTimePlotAddon* m_plotAddon;
	GRDeviceAddon* m_dev;
	QPen m_pen;

	PlotChannel *m_plotCh;
	PlotAxis *m_plotAxis;
	PlotAxisHandle *m_plotAxisHandle;

	QString name;
	QWidget *widget;
};
}
#endif // GRTIMECHANNELADDON_H
