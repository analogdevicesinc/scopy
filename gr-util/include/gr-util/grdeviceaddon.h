#ifndef GRDEVICEADDON_H
#define GRDEVICEADDON_H

#include "tooladdon.h"
#include "griiodevicesource.h"
#include <QLabel>
#include "scopy-gr-util_export.h"

namespace scopy::grutil {
class GRTimeChannelAddon;
class SCOPY_GR_UTIL_EXPORT GRDeviceAddon : public QObject, public ToolAddon {
	Q_OBJECT
public:
	GRDeviceAddon(GRIIODeviceSource *src, QObject *parent = nullptr);
	~GRDeviceAddon();

	QString getName() override;
	QWidget* getWidget() override;

	GRIIODeviceSource *src() const;

	void registerChannel(GRTimeChannelAddon *ch);
	QList<GRTimeChannelAddon*> getRegisteredChannels();

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
	QString name;
	QWidget *widget;
	GRIIODeviceSource *m_src;
	QList<GRTimeChannelAddon*> m_channels;
};
}

#endif // GRDEVICEADDON_H
