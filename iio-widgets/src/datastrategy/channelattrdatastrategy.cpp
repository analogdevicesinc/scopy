#include "datastrategy/channelattrdatastrategy.h"
#include <QFile>

#define BUFFER_SIZE 256

Q_LOGGING_CATEGORY(CAT_ATTR_DATA_STRATEGY, "AttrDataStrategy")

using namespace scopy::attr;

ChannelAttrDataStrategy::ChannelAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent)
{
	m_recipe = recipe;
	setParent(parent);
}

void ChannelAttrDataStrategy::save(QString data)
{
	if(m_recipe.channel == nullptr || m_recipe.data == "") {
		qWarning(CAT_ATTR_DATA_STRATEGY) << "Invalid arguments, cannot write any data";
		return;
	}

	ssize_t res = iio_channel_attr_write(m_recipe.channel, m_recipe.data.toStdString().c_str(),
					     data.toStdString().c_str());
	if(res < 0) {
		qWarning(CAT_ATTR_DATA_STRATEGY) << "Cannot write" << data << "to" << m_recipe.data;
	}

	Q_EMIT emitStatus((int)(res));
	requestData(); // readback
}

void ChannelAttrDataStrategy::requestData()
{
	if(m_recipe.channel == nullptr || m_recipe.data.isEmpty()) {
		qWarning(CAT_ATTR_DATA_STRATEGY) << "Invalid arguments, cannot read any data";
		return;
	}

	char options[BUFFER_SIZE] = {0}, currentValue[BUFFER_SIZE] = {0};

	ssize_t currentValueResult =
		iio_channel_attr_read(m_recipe.channel, m_recipe.data.toStdString().c_str(), currentValue, BUFFER_SIZE);
	if(currentValueResult < 0) {
		qWarning(CAT_ATTR_DATA_STRATEGY)
			<< "Could not read" << m_recipe.data << "error code:" << currentValueResult;
	}

	if(m_recipe.dataOptions != "") {
		ssize_t optionsResult = iio_channel_attr_read(
			m_recipe.channel, m_recipe.dataOptions.toStdString().c_str(), options, BUFFER_SIZE);
		if(optionsResult < 0) {
			qWarning(CAT_ATTR_DATA_STRATEGY)
				<< "Could not read" << m_recipe.data << "error code:" << optionsResult;
		}
	}

	Q_EMIT sendData(QString(currentValue), QString(options));
}

#include "moc_channelattrdatastrategy.cpp"
