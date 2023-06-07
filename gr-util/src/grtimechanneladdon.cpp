#include "grtimechanneladdon.h"
#include "grdeviceaddon.h"

using namespace scopy::grutil;

GRTimeChannelAddon::GRTimeChannelAddon(GRSignalPath *path, GRTimePlotAddon *plotAddon, QObject *parent) : m_plotAddon(plotAddon) {
	this->ch = dynamic_cast<GRIIOChannel*>(path->path()[0]);
	name = ch->getChannelName();
	widget = new QLabel(name);
}

GRTimeChannelAddon::~GRTimeChannelAddon() {}

QString GRTimeChannelAddon::getName() {return name;}

QWidget *GRTimeChannelAddon::getWidget() {return widget;}

void GRTimeChannelAddon::setDevice(GRDeviceAddon *d) { m_dev = d; d->registerChannel(this);}

GRDeviceAddon* GRTimeChannelAddon::getDevice() { return m_dev;}

void GRTimeChannelAddon::enable() {}

void GRTimeChannelAddon::disable() {}

void GRTimeChannelAddon::onStart() {}

void GRTimeChannelAddon::onStop() {}

void GRTimeChannelAddon::onAdd() {  }

void GRTimeChannelAddon::onRemove() {}

void GRTimeChannelAddon::onChannelAdded(ToolAddon *) {}

void GRTimeChannelAddon::onChannelRemoved(ToolAddon *) {}
