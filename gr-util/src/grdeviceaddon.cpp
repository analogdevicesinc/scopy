#include "grdeviceaddon.h"

#include <QDebug>

using namespace scopy::grutil;

GRDeviceAddon::GRDeviceAddon(GRIIODeviceSource *src, QObject *parent)
	: QObject(parent)
{
	name = src->deviceName();
	widget = new QLabel("devicename" + src->deviceName());
	m_src = src;
	connect(this, &GRDeviceAddon::updateBufferSize, this, &GRDeviceAddon::setBufferSize);
}

GRDeviceAddon::~GRDeviceAddon() {}

QString GRDeviceAddon::getName() { return name; }

QWidget *GRDeviceAddon::getWidget() { return widget; }

GRIIODeviceSource *GRDeviceAddon::src() const { return m_src; }

void GRDeviceAddon::enable() {}

void GRDeviceAddon::disable() {}

void GRDeviceAddon::onStart() {}

void GRDeviceAddon::onStop() {}

void GRDeviceAddon::onInit() {}

void GRDeviceAddon::onDeinit() {}

void GRDeviceAddon::preFlowStart() {}

void GRDeviceAddon::postFlowStart() {}

void GRDeviceAddon::preFlowBuild() {}

void GRDeviceAddon::onChannelAdded(ToolAddon *) {}

void GRDeviceAddon::onChannelRemoved(ToolAddon *) {}

void GRDeviceAddon::setBufferSize(uint32_t bufferSize) { m_src->setBuffersize(bufferSize); }

void GRDeviceAddon::registerChannel(GRTimeChannelAddon *ch) { m_channels.append(ch); }

QList<GRTimeChannelAddon *> GRDeviceAddon::getRegisteredChannels() { return m_channels; }
