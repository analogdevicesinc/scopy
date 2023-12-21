#include "datastrategy/deviceattrdatastrategy.h"
#define BUFFER_SIZE 16384

Q_LOGGING_CATEGORY(CAT_DEVICE_DATA_STRATEGY, "DeviceDataStrategy")
using namespace scopy::attr;

DeviceAttrDataStrategy::DeviceAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent)
{
	m_recipe = recipe;
	setParent(parent);
}

void DeviceAttrDataStrategy::save(QString data)
{
	if(m_recipe.device == nullptr || m_recipe.data == "") {
		qWarning(CAT_DEVICE_DATA_STRATEGY) << "Invalid arguments, cannot write any data";
		return;
	}

	ssize_t res =
		iio_device_attr_write(m_recipe.device, m_recipe.data.toStdString().c_str(), data.toStdString().c_str());
	if(res < 0) {
		qWarning(CAT_DEVICE_DATA_STRATEGY) << "Cannot write" << data << "to" << m_recipe.data;
	}

	Q_EMIT emitStatus((int)(res));
	requestData();
}

void DeviceAttrDataStrategy::requestData()
{
	if(m_recipe.device == nullptr || m_recipe.data.isEmpty()) {
		qWarning(CAT_DEVICE_DATA_STRATEGY) << "Invalid arguments, cannot read any data";
		return;
	}

	char options[BUFFER_SIZE] = {0}, currentValue[BUFFER_SIZE] = {0};
	ssize_t currentValueResult =
		iio_device_attr_read(m_recipe.device, m_recipe.data.toStdString().c_str(), currentValue, BUFFER_SIZE);
	if(currentValueResult < 0) {
		qWarning(CAT_DEVICE_DATA_STRATEGY)
			<< "Could not read" << m_recipe.data << "error code:" << currentValueResult;
	}

	if(m_recipe.dataOptions != "") {
		ssize_t optionsResult = iio_device_attr_read(
			m_recipe.device, m_recipe.dataOptions.toStdString().c_str(), options, BUFFER_SIZE);
		if(optionsResult < 0) {
			qWarning(CAT_DEVICE_DATA_STRATEGY)
				<< "Could not read" << m_recipe.data << "error code:" << optionsResult;
		}
	}

	Q_EMIT sendData(QString(currentValue), QString(options));
}

#include "moc_deviceattrdatastrategy.cpp"
