#include "grdeviceaddon.h"

#include "grtimechanneladdon.h"

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

void GRDeviceAddon::enable()
{
	for(auto ch : getRegisteredChannels()) {
		ch->enable();
	}
}

void GRDeviceAddon::disable()
{
	for(auto ch : getRegisteredChannels()) {
		ch->disable();
	}
}

void GRDeviceAddon::onStart()
{
	for(auto ch : getRegisteredChannels()) {
		ch->onStart();
	}
}

void GRDeviceAddon::onStop()
{
	for(auto ch : getRegisteredChannels()) {
		ch->onStop();
	}
}

void GRDeviceAddon::onInit()
{
	for(auto ch : getRegisteredChannels()) {
		ch->onInit();
	}
}

void GRDeviceAddon::onDeinit()
{
	for(auto ch : getRegisteredChannels()) {
		ch->onDeinit();
	}
}

void GRDeviceAddon::preFlowStart()
{
	for(auto ch : getRegisteredChannels()) {
		ch->preFlowStart();
	}
}

void GRDeviceAddon::postFlowStart()
{
	for(auto ch : getRegisteredChannels()) {
		ch->postFlowStart();
	}
}

void GRDeviceAddon::preFlowStop()
{
	for(auto ch : getRegisteredChannels()) {
		ch->preFlowStop();
	}
}

void GRDeviceAddon::postFlowStop()
{
	for(auto ch : getRegisteredChannels()) {
		ch->postFlowStop();
	}
}

void GRDeviceAddon::preFlowBuild()
{
	for(auto ch : getRegisteredChannels()) {
		ch->preFlowBuild();
	}
}

void GRDeviceAddon::postFlowBuild()
{
	for(auto ch : getRegisteredChannels()) {
		ch->postFlowBuild();
	}
}

void GRDeviceAddon::preFlowTeardown()
{
	for(auto ch : getRegisteredChannels()) {
		ch->preFlowTeardown();
	}
}

void GRDeviceAddon::postFlowTeardown()
{
	for(auto ch : getRegisteredChannels()) {
		ch->postFlowTeardown();
	}
}

void GRDeviceAddon::setBufferSize(uint32_t bufferSize) { m_src->setBuffersize(bufferSize); }

void GRDeviceAddon::registerChannel(GRTimeChannelAddon *ch) { m_channels.append(ch); }

QList<GRTimeChannelAddon *> GRDeviceAddon::getRegisteredChannels() { return m_channels; }
