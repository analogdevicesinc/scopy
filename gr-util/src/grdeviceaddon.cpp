#include "grdeviceaddon.h"


using namespace scopy::grutil;

GRDeviceAddon::GRDeviceAddon(GRIIODeviceSource *src, QObject *parent) {
	name = src->deviceName();
	widget = new QLabel("devicename" + src->deviceName());
	m_src = src;
}

GRDeviceAddon::~GRDeviceAddon() {}

QString GRDeviceAddon::getName() { return name;}

QWidget *GRDeviceAddon::getWidget() { return widget;}

void GRDeviceAddon::enable() {}

void GRDeviceAddon::disable() {}

void GRDeviceAddon::onStart() {}

void GRDeviceAddon::onStop() {}

void GRDeviceAddon::onAdd() {}

void GRDeviceAddon::onRemove() {}

void GRDeviceAddon::onChannelAdded(ToolAddon *) {}

void GRDeviceAddon::onChannelRemoved(ToolAddon *) {}

void GRDeviceAddon::registerChannel(GRTimeChannelAddon* ch) {
	m_channels.append(ch);
}

QList<GRTimeChannelAddon *> GRDeviceAddon::getRegisteredChannels() {
	return m_channels;
}




