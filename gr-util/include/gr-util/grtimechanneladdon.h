#ifndef GRTIMECHANNELADDON_H
#define GRTIMECHANNELADDON_H

#include "tooladdon.h"
#include "grsignalpath.h"
#include "grtimeplotaddon.h"
#include "griiodevicesource.h"
#include <QLabel>
#include "scopy-gr-util_export.h"

namespace scopy::grutil {
class GRDeviceAddon;
class SCOPY_GR_UTIL_EXPORT GRTimeChannelAddon : public QObject, public ToolAddon {
	Q_OBJECT
public:
	GRTimeChannelAddon(GRSignalPath* path, GRTimePlotAddon* plotAddon, QObject *parent = nullptr);
	~GRTimeChannelAddon();

	QString getName() override;
	QWidget* getWidget() override;
	void setDevice(GRDeviceAddon *d);
	GRDeviceAddon* getDevice();

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
	GRIIOChannel* ch;
	GRTimePlotAddon* m_plotAddon;
	GRDeviceAddon* m_dev;
	QString name;
	QWidget *widget;
};
}
#endif // GRTIMECHANNELADDON_H
