#include "iiostandarditem.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_IIOSTANDARDITEM, "IIOStandardItem")

using namespace scopy::iiodebugplugin;

IIOStandardItem::IIOStandardItem(QList<IIOWidget *> widgets, QString name, QString path, IIOStandardItem::Type type)
	: QStandardItem(name)
	, m_device(nullptr)
	, m_channel(nullptr)
	, m_iioWidgets(widgets)
	, m_id()
	, m_name(name)
	, m_path(path)
	, m_format()
	, m_triggerName()
	, m_triggerStatus()
	, m_isWatched(false)
	, m_type(type)
	, m_isBufferCapable(false)
	, m_index(-1)
{
	generateToolTip();
}

IIOStandardItem::IIOStandardItem(QList<IIOWidget *> widgets, QString name, QString id, QString path, Type type)
	: QStandardItem((!id.isEmpty()) ? id + ": " + name : name)
	, m_device(nullptr)
	, m_channel(nullptr)
	, m_iioWidgets(widgets)
	, m_id(id)
	, m_name(name)
	, m_path(path)
	, m_format()
	, m_triggerName()
	, m_triggerStatus()
	, m_isWatched(false)
	, m_type(type)
	, m_isBufferCapable(false)
	, m_index(-1)
{
	generateToolTip();
}

void IIOStandardItem::setDevice(struct iio_device *device)
{
	m_device = device;
	extractDataFromDevice();
}

void IIOStandardItem::setChannel(struct iio_channel *channel)
{
	m_channel = channel;
	extractDataFromChannel();
}

IIOStandardItem::~IIOStandardItem() = default;

QList<scopy::IIOWidget *> IIOStandardItem::getIIOWidgets() { return m_iioWidgets; }

QStringList IIOStandardItem::details() { return m_details; }

QString IIOStandardItem::id() { return m_id; }

QString IIOStandardItem::name() { return m_name; }

QString IIOStandardItem::path() { return m_path; }

QString IIOStandardItem::format() { return m_format; }

QString IIOStandardItem::trigger() { return m_triggerName; }

QString IIOStandardItem::triggerStatus() { return m_triggerStatus; }

IIOStandardItem::Type IIOStandardItem::type() { return m_type; }

int IIOStandardItem::index() { return m_index; }

bool IIOStandardItem::isScanElement() { return m_isScanElement; }

bool IIOStandardItem::isOutput() { return m_isOutput; }

bool IIOStandardItem::isEnabled() { return m_isEnabled; }

bool IIOStandardItem::isBufferCapable() { return m_isBufferCapable; }

void IIOStandardItem::setBufferCapable(bool isBufferCapable)
{
	if(m_type == IIOStandardItem::Device) {
		m_isBufferCapable = isBufferCapable;
	} else {
		qWarning(CAT_IIOSTANDARDITEM)
			<< "The current IIOStandardItem is not a Device, cannot set isBufferCapable flag.";
	}
}

bool IIOStandardItem::isWatched() { return m_isWatched; }

void IIOStandardItem::setWatched(bool isWatched) { m_isWatched = isWatched; }

void IIOStandardItem::buildDetails()
{
	if(!m_iioWidgets.empty()) {
		// if there is no channel/device provided, this should get the necessary data
		// only works if there are channel/device attributes, otherwise iio-widgets are not created
		auto widget = m_iioWidgets[0];
		struct iio_channel *channel = widget->getRecipe().channel;
		struct iio_device *device = widget->getRecipe().device;
		if(channel != nullptr) {
			m_channel = channel;
			extractDataFromChannel();
		}

		if(device != nullptr) {
			m_device = device;
			extractDataFromDevice();
		}
	}
}

void IIOStandardItem::generateToolTip()
{
	switch(m_type) {
	case IIOStandardItem::Context: {
		setToolTip("Context");
		break;
	}
	case IIOStandardItem::ContextAttribute: {
		setToolTip("Context Attribute");
		break;
	}
	case IIOStandardItem::Trigger: {
		setToolTip("Trigger");
		break;
	}
	case IIOStandardItem::Device: {
		setToolTip("Device");
		break;
	}
	case IIOStandardItem::DeviceAttribute: {
		setToolTip("Device Attribute");
		break;
	}
	case IIOStandardItem::Channel: {
		setToolTip("Channel");
		break;
	}
	case IIOStandardItem::ChannelAttribute: {
		setToolTip("Channel Attribute");
		break;
	}
	default:
		break;
	}
}

void IIOStandardItem::extractDataFromDevice()
{
	bool is_hwmon = iio_device_is_hwmon(m_device);
	m_details.append((is_hwmon) ? "This device is hardware monitor." : "This device is NOT hardware monitor.");

	bool is_trigger = iio_device_is_trigger(m_device);
	m_details.append((is_trigger) ? "This device is trigger." : "This device is NOT trigger.");

	const struct iio_device **trig;
	int ret = iio_device_get_trigger(m_device, trig);
	if(ret == 0) {
		m_triggerName = iio_device_get_name(*trig);
		m_triggerStatus = QString("Current trigger: %1(%2)").arg(iio_device_get_id(*trig)).arg(m_triggerName);
	} else if(ret == -ENODEV) {
		m_triggerStatus = "No trigger assigned on this device";
	} else if(ret == -ENOENT) {
		m_triggerStatus = "No trigger on this device";
	} else if(ret < 0) {
		m_triggerStatus = "Unable to get trigger";
	}
	m_details.append(m_triggerStatus + ".");
}

void IIOStandardItem::extractDataFromChannel()
{
	m_isScanElement = iio_channel_is_scan_element(m_channel);
	m_details.append((m_isScanElement) ? "This channel is scan element." : "This channel is NOT scan element.");

	m_isOutput = iio_channel_is_output(m_channel);
	m_details.append((m_isOutput) ? "This channel is output." : "This channel is input.");

	m_isEnabled = iio_channel_is_enabled(m_channel);
	m_details.append((m_isEnabled) ? "This channel is enabled." : "This channel is not enabled.");

	if(m_isScanElement) {
		const struct iio_data_format *format = iio_channel_get_data_format(m_channel);
		char sign = format->is_signed ? 's' : 'u';
		// magic number from
		// https://github.com/analogdevicesinc/libiio/blob/35a0e4d3d45990a5c3baa7add154c15f6b6a4401/utils/iio_info.c#L122
		char repeat[12];
		repeat[0] = '\0';

		if(format->is_fully_defined) {
			sign += 'A' - 'a';
		}

		if(format->repeat > 1) {
			snprintf(repeat, sizeof(repeat), "X%u", format->repeat);
		}

		m_format = QString("%1e:%2%3/%4%5>>%6")
				   .arg(format->is_be ? 'b' : 'l')
				   .arg(sign)
				   .arg(format->bits)
				   .arg(format->length)
				   .arg(repeat)
				   .arg(format->shift);
		m_index = iio_channel_get_index(m_channel);
	}
}
