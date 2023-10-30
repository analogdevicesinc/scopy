#include "grdeviceaddon.h"
#include <utility>

using namespace scopy::grutil;

GRDeviceAddon::GRDeviceAddon(GRIIODeviceSource *src, QObject *parent)
	: QObject(parent)
{
	name = src->deviceName();
	auto triggerController = new TriggerController(src, this);
	widget = triggerController->getTriggerMenuSection();
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

TriggerController::TriggerController(GRIIODeviceSource *src, QObject *parent)
	: QObject(parent)
	, m_triggerHandler(src->getTriggerHandler())
	, m_triggerMenuSection(new TriggerMenuSection())
{
	auto triggerNames = m_triggerHandler->getAvailableTriggerNames();
	for(const QString &trigger : triggerNames) {
		m_triggerMenuSection->addTrigger(trigger);
	}
	QString currentTrigger = m_triggerHandler->getTriggerFromDevice(src->deviceName());
	m_triggerMenuSection->selectTrigger(currentTrigger);

	connect(m_triggerMenuSection, &TriggerMenuSection::selectedTrigger, this, [this, src](QString triggerName) {
		m_triggerHandler->setTrigger(src->deviceName(), std::move(triggerName));
	});
}

scopy::TriggerHandler *TriggerController::getTriggerHandler() { return m_triggerHandler; }

scopy::TriggerMenuSection *TriggerController::getTriggerMenuSection() { return m_triggerMenuSection; }
