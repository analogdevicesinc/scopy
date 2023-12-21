#include "grdeviceaddon.h"
#include "menusectionwidget.h"
#include "menucollapsesection.h"
#include "menuheader.h"
#include "errorbox.h"
#include <iio-widgets/iiowidgetfactory.h>
#include <iio-widgets/iiowidget.h>

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
	IIOWidgetFactory *attrFactory = new IIOWidgetFactory(attrContainer);
	QList<IIOWidget *> attrWidgets = attrFactory->buildAllAttrsForDevice(m_src->iioDev());
	const struct iio_context *ctx = iio_device_get_context(m_src->iioDev());
	attrWidgets.append(attrFactory->buildSingle(
		IIOWidgetFactory::ExternalSave | IIOWidgetFactory::ComboUi | IIOWidgetFactory::TriggerData,
		{.context = const_cast<iio_context *>(ctx), .device = m_src->iioDev(), .data = "Triggers"}));

	auto layout = new QVBoxLayout(attrContainer);
	layout->setSpacing(10);
	layout->setMargin(0);

	for(auto w : attrWidgets) {
		auto container = new QWidget(attr);
		auto errBox = new ErrorBox(container);
		container->setLayout(new QHBoxLayout(container));
		container->layout()->addWidget(w);
		container->layout()->addWidget(errBox);
		connect(w, &IIOWidget::currentStateChanged, this,
			[errBox](IIOWidget::State state, QString explanation) {
				switch(state) {
				case IIOWidget::Busy:
					errBox->changeColor(ErrorBox::Yellow);
					break;
				case IIOWidget::Correct:
					errBox->changeColor(ErrorBox::Green);
					break;
				case IIOWidget::Error:
					errBox->changeColor(ErrorBox::Red);
					break;
				}
				errBox->setToolTip(explanation);
			});

		layout->addWidget(container);
	}

	attr->contentLayout()->addLayout(layout);
	attrContainer->contentLayout()->addWidget(attr);
	attr->header()->setChecked(false);
	return attrContainer;
}

QWidget *GRDeviceAddon::createMenu(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);

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
