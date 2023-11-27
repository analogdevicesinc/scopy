#include "datastrategy/attrdatastrategy.h"
#include <utility>
#include <QFile>

Q_LOGGING_CATEGORY(CAT_ATTR_DATA_STRATEGY, "AttrDataStrategy")

using namespace scopy::attr;

AttrDataStrategy::AttrDataStrategy(AttributeFactoryRecipe recipe, QObject *parent)
{
	m_recipe = std::move(recipe);
	setParent(parent);
}

void AttrDataStrategy::save(QString data)
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

void AttrDataStrategy::requestData()
{
	if(m_recipe.channel == nullptr || m_recipe.data.isEmpty()) {
		qWarning(CAT_ATTR_DATA_STRATEGY) << "Invalid arguments, cannot read any data";
		return;
	}

	char options[256] = {0}, currentValue[256] = {0};

	ssize_t currentValueResult =
		iio_channel_attr_read(m_recipe.channel, m_recipe.data.toStdString().c_str(), currentValue, 256);
	if(currentValueResult < 0) {
		qWarning(CAT_ATTR_DATA_STRATEGY)
			<< "Could not read" << m_recipe.data << "error code:" << currentValueResult;
	}

	if(m_recipe.dataOptions != "") {
		ssize_t optionsResult = iio_channel_attr_read(m_recipe.channel,
							      m_recipe.dataOptions.toStdString().c_str(), options, 256);
		if(optionsResult < 0) {
			qWarning(CAT_ATTR_DATA_STRATEGY)
				<< "Could not read" << m_recipe.data << "error code:" << optionsResult;
		}
	}

	Q_EMIT sendData(QString(currentValue), QString(options));
}

#include "moc_attrdatastrategy.cpp"
