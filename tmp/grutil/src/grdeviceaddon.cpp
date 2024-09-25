#include "grdeviceaddon.h"
#include "menusectionwidget.h"
#include "menucollapsesection.h"
#include "menuheader.h"
#include "errorbox.h"
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/iiowidget.h>

#include "grtimechanneladdon.h"

#include <QDebug>

using namespace scopy::grutil;

GRDeviceAddon::GRDeviceAddon(GRIIODeviceSource *src, QObject *parent)
	: QObject(parent)
{
	name = src->deviceName();
	m_pen = QPen(StyleHelper::getColor("ScopyBlue"));
	m_src = src;
	connect(this, &GRDeviceAddon::updateBufferSize, this, &GRDeviceAddon::setBufferSize);
	widget = createMenu();
}

QWidget *GRDeviceAddon::createAttrMenu(QWidget *parent)
{
	MenuSectionWidget *attrContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *attr =
		new MenuCollapseSection("ATTRIBUTES", MenuCollapseSection::MHCW_NONE, attrContainer);
	QList<IIOWidget *> attrWidgets = IIOWidgetBuilder().device(m_src->iioDev()).parent(parent).buildAll();
	const struct iio_context *ctx = iio_device_get_context(m_src->iioDev());
	attrWidgets.append(IIOWidgetBuilder()
				   .context(const_cast<iio_context *>(ctx))
				   .device(m_src->iioDev())
				   .attribute("Triggers")
				   .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
				   .dataStrategy(IIOWidgetBuilder::DS::TriggerData)
				   .parent(parent)
				   .buildSingle());

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(0, 0, 0, 10); // bottom margin
	layout->setMargin(0);

	for(auto w : attrWidgets) {
		layout->addWidget(w);
	}

	attr->contentLayout()->addLayout(layout);
	attrContainer->contentLayout()->addWidget(attr);
	attr->header()->setChecked(false);
	return attrContainer;
}

QWidget *GRDeviceAddon::createMenu(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout();

	QScrollArea *scroll = new QScrollArea(parent);
	QWidget *wScroll = new QWidget(scroll);
	QVBoxLayout *layScroll = new QVBoxLayout(wScroll);
	layScroll->setMargin(0);
	layScroll->setSpacing(10);

	wScroll->setLayout(layScroll);
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	scroll->setWidget(wScroll);

	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	MenuHeaderWidget *header = new MenuHeaderWidget(name, m_pen, w);
	QWidget *attrMenu = createAttrMenu(w);

	lay->addWidget(header);
	lay->addWidget(scroll);
	layScroll->addWidget(attrMenu);

	layScroll->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	return w;
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

#include "moc_grdeviceaddon.cpp"
