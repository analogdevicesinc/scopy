#include "clidetailsview.h"
#include "debuggerloggingcategories.h"

using namespace scopy::iiodebugplugin;

CliDetailsView::CliDetailsView(QWidget *parent)
	: QWidget(parent)
	, m_textBrowser(new QTextBrowser(this))
	, m_currentText(QString())
	, m_noCtxAttributes(0)
	, m_noDevices(0)
	, m_noDevAttributes(0)
	, m_noChannels(0)
	, m_noChnlAttributes(0)
{
	setupUi();
}

void CliDetailsView::setIIOStandardItem(IIOStandardItem *item)
{
	// TODO: add validation (big time)
	m_currentItem = item;
	m_currentText.clear();
	m_deviceAttrsString.clear();
	m_noCtxAttributes = 0;
	m_noDevices = 0;
	m_noDevAttributes = 0;
	m_noChannels = 0;
	m_noChnlAttributes = 0;

	switch(m_currentItem->type()) {
	case IIOStandardItem::Context:
		m_globalLevel = 0;
		setupContext();
		break;
	case IIOStandardItem::ContextAttribute:
		m_globalLevel = 1;
		m_contextIIOItem = item;
		setupContextAttr();
		break;
	case IIOStandardItem::Device:
	case IIOStandardItem::Trigger:
		m_globalLevel = 1;
		m_contextIIOItem = item;
		setupDevice();
		break;
	case IIOStandardItem::DeviceAttribute:
		m_globalLevel = 4;
		m_deviceIIOItem = item;
		setupDeviceAttr();
		m_currentText = m_deviceAttrsString;
		break;
	case IIOStandardItem::Channel:
		m_globalLevel = 3;
		m_deviceIIOItem = item;
		setupChannel();
		break;
	case IIOStandardItem::ChannelAttribute:
		m_globalLevel = 4;
		m_channelIIOItem = item;
		setupChannelAttr();
		break;
	default:
		break;
	}

	m_textBrowser->setText(m_currentText);
}

void CliDetailsView::refreshView() { setIIOStandardItem(m_currentItem); }

void CliDetailsView::setupUi()
{
	setContentsMargins(0, 0, 0, 0);
	setLayout(new QVBoxLayout(this));
	layout()->addWidget(m_textBrowser);
	layout()->setContentsMargins(0, 0, 0, 0);
	m_textBrowser->setTabStopDistance(30);
	QFont mono("Monospace");
	mono.setStyleHint(QFont::Monospace);
	m_textBrowser->setFont(mono);
}

void CliDetailsView::setupChannelAttr()
{
	IIOWidget *w = m_channelIIOItem->getIIOWidgets()[0];
	DataStrategyInterface *ds = w->getDataStrategy();

	m_currentText.append(tabs(4) + "attr " + QString::number(m_noChnlAttributes) + ": " + m_channelIIOItem->name() +
			     " value: " + ds->data() + "\n");
	++m_noChnlAttributes;
	QString channelOptData = ds->optionalData();
	if(!channelOptData.isEmpty()) {
		m_currentText.append(tabs(4) + "attr " + QString::number(m_noChnlAttributes) + ": " +
				     w->getRecipe().iioDataOptions + " value: " + channelOptData + "\n");
		++m_noChnlAttributes;
	}
}

void CliDetailsView::setupChannel()
{
	++m_noChannels;
	m_currentText.append(
		tabs(3) + m_deviceIIOItem->text() + ": (" + (m_deviceIIOItem->isOutput() ? "output" : "input") +
		(m_deviceIIOItem->isScanElement() ? ", index: " + QString::number(m_deviceIIOItem->index()) +
				 ", format: " + m_deviceIIOItem->format()
						  : "") +
		")\n");

	int channelChildrenCount = m_deviceIIOItem->rowCount();
	for(int i = 0; i < channelChildrenCount; ++i) {
		QStandardItem *channelChild = m_deviceIIOItem->child(i);
		m_channelIIOItem = dynamic_cast<IIOStandardItem *>(channelChild);

		// the item is for sure a ChannelAttribute
		if(m_noChnlAttributes == 0) {
			m_currentText.append(tabs(3) + "%%CHANNEL_ATTRS_COUNT%% channel-specific attributes found:\n");
		}
		setupChannelAttr();
	}
	m_currentText.replace("%%CHANNEL_ATTRS_COUNT%%", QString::number(m_noChnlAttributes));
	m_noChnlAttributes = 0;
}

void CliDetailsView::setupDeviceAttr()
{
	IIOWidget *w = m_deviceIIOItem->getIIOWidgets()[0];
	DataStrategyInterface *ds = w->getDataStrategy();

	m_deviceAttrsString.append(tabs(4) + "attr " + QString::number(m_noDevAttributes) + ": " +
				   m_deviceIIOItem->name() + " value: " + ds->data() + "\n");
	++m_noDevAttributes;

	QString deviceOptData = ds->optionalData();
	if(!deviceOptData.isEmpty()) {
		m_deviceAttrsString.append(tabs(4) + "attr " + QString::number(m_noDevAttributes) + ": " +
					   w->getRecipe().iioDataOptions + " value: " + deviceOptData + "\n");
		++m_noDevAttributes;
	}
}

void CliDetailsView::setupDevice()
{
	++m_noDevices;
	m_currentText.append(tabs(1) + m_contextIIOItem->text() +
			     ((m_contextIIOItem->isBufferCapable()) ? " (buffer capable)" : "") + "\n");

	int deviceChildrenCount = m_contextIIOItem->rowCount();
	m_currentText.append(tabs(2) + "%%CHANNELS_COUNT%% channels found:\n");

	m_deviceAttrsString = "";
	for(int i = 0; i < deviceChildrenCount; ++i) {
		QStandardItem *deviceChild = m_contextIIOItem->child(i);
		m_deviceIIOItem = dynamic_cast<IIOStandardItem *>(deviceChild);

		if(m_deviceIIOItem->type() == IIOStandardItem::Channel) {
			setupChannel();
		} else if(m_deviceIIOItem->type() == IIOStandardItem::DeviceAttribute) {

			if(m_noDevAttributes == 0) {
				m_deviceAttrsString.append(tabs(2) +
							   "%%DEV_ATTRS_COUNT%% device-specific attributes found:\n");
			}

			setupDeviceAttr();
		} else {
			qWarning(CAT_DETAILSVIEW)
				<< "Error when setting up the device, the type is not Channel nor DeviceAttribute";
		}
	}
	m_currentText.append(m_deviceAttrsString);
	m_currentText.append(tabs(2) + m_contextIIOItem->triggerStatus() + "\n");

	m_currentText.replace("%%CHANNELS_COUNT%%", QString::number(m_noChannels));
	m_noChannels = 0;

	m_currentText.replace("%%DEV_ATTRS_COUNT%%", QString::number(m_noDevAttributes));
	m_noDevAttributes = 0;
}

void CliDetailsView::setupContextAttr()
{
	++m_noCtxAttributes;
	m_currentText.append(tabs(1) + m_contextIIOItem->name() + ": " +
			     m_contextIIOItem->getIIOWidgets()[0]->getDataStrategy()->data() + "\n");
}

void CliDetailsView::setupContext()
{
	m_currentText.append("IIO context has %%CTX_ATTRS%% attributes:\n");
	int childrenCount = m_currentItem->rowCount();
	for(int i = 0; i < childrenCount; ++i) {
		QStandardItem *child = m_currentItem->child(i);
		m_contextIIOItem = dynamic_cast<IIOStandardItem *>(child);

		// we consider that the list of children is divided in 2 sections: the
		// attributes and the devices/triggers and they cannot be interleaved
		if(m_contextIIOItem->type() == IIOStandardItem::ContextAttribute) {
			setupContextAttr();
		} else if(m_contextIIOItem->type() == IIOStandardItem::Device ||
			  m_contextIIOItem->type() == IIOStandardItem::Trigger) {
			if(m_noDevices == 0) {
				m_currentText.append("IIO context has %%DEVICES_COUNT%% devices:\n");
			}
			setupDevice();
		}
	}
	m_currentText.replace("%%CTX_ATTRS%%", QString::number(m_noCtxAttributes));
	m_currentText.replace("%%DEVICES_COUNT%%", QString::number(m_noDevices));
}

QString CliDetailsView::tabs(int level)
{
	switch(level - m_globalLevel) {
	case 0:
		return "";
	case 1:
		return "\t";
	case 2:
		return "\t\t";
	case 3:
		return "\t\t\t";
	case 4:
		return "\t\t\t\t";
	default:
		return "";
	}
}
