#include "grdevicecomponent.h"
#include "src/channelcomponent.h"
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>
#include <widgets/menuheader.h>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetbuilder.h>

using namespace scopy;
using namespace scopy::adc;
// using namespace scopy::gui;
using namespace scopy::grutil;

GRDeviceComponent::GRDeviceComponent(GRIIODeviceSourceNode *node, QWidget *parent)
	: QWidget(parent)
	, ToolComponent()
{
	m_node = node;
	name = node->name();
	m_pen = QPen(StyleHelper::getColor("ScopyBlue"));
	m_src = node->src();
	// connect(this, &GRDeviceAddon::updateBufferSize, this, &GRDeviceAddon::setBufferSize);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	widget = createMenu(this);
	m_lay->addWidget(widget);
	setLayout(m_lay);
	createMenuControlButton();
}

QWidget *GRDeviceComponent::createAttrMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *attr =
		new MenuSectionCollapseWidget("ATTRIBUTES", MenuCollapseSection::MHCW_NONE, parent);

	QList<IIOWidget *> attrWidgets = IIOWidgetBuilder().device(m_src->iioDev()).buildAll();
	const struct iio_context *ctx = iio_device_get_context(m_src->iioDev());
	attrWidgets.append(IIOWidgetBuilder()
				   .context(const_cast<iio_context *>(ctx))
				   .device(m_src->iioDev())
				   .attribute("Triggers")
				   .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
				   .dataStrategy(IIOWidgetBuilder::DS::TriggerData)
				   .buildSingle());

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(0, 0, 0, 10); // bottom margin
	layout->setMargin(0);

	for(auto w : attrWidgets) {
		layout->addWidget(w);
	}

	attr->contentLayout()->addLayout(layout);
	attr->setCollapsed(true);
	return attr;
}

QWidget *GRDeviceComponent::createMenu(QWidget *parent)
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

void GRDeviceComponent::createMenuControlButton(QWidget *parent)
{
	m_ctrl = new CollapsableMenuControlButton(parent);
	setupDeviceMenuControlButtonHelper(m_ctrl->getControlBtn(), name);
}

void GRDeviceComponent::setupDeviceMenuControlButtonHelper(MenuControlButton *devBtn, QString name)
{
	devBtn->setName(name);
	devBtn->setCheckable(true);
	devBtn->button()->setVisible(false);
	devBtn->setOpenMenuChecksThis(true);
	devBtn->setDoubleClickToOpenMenu(true);
}

CollapsableMenuControlButton *GRDeviceComponent::ctrl() { return m_ctrl; }

bool GRDeviceComponent::sampleRateAvailable()
{
	for(auto c : qAsConst(m_channels)) {
		if(c->enabled())
			return m_src->sampleRateAvailable();
	}
	return false;
}

double GRDeviceComponent::sampleRate() { return m_src->readSampleRate(); }

void GRDeviceComponent::setBufferSize(uint32_t bufferSize) { m_src->setBuffersize(bufferSize); }

GRDeviceComponent::~GRDeviceComponent() {}

void GRDeviceComponent::onStart() {}

void GRDeviceComponent::onStop() {}

void GRDeviceComponent::onInit() {}

void GRDeviceComponent::onDeinit() {}

void GRDeviceComponent::addChannel(ChannelComponent *c) { m_channels.append(c); }

void GRDeviceComponent::removeChannel(ChannelComponent *c) { m_channels.removeAll(c); }
