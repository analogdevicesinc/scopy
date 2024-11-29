#include "datastrategy/triggerdatastrategy.h"
#include <utility>
#include <QFile>

using namespace scopy::attr;

Q_LOGGING_CATEGORY(CAT_TRIGGER_DATA_STRATEGY, "TriggerDataStrategy")

TriggerDataStrategy::TriggerDataStrategy(AttributeFactoryRecipe recipe, QObject *parent)
{
	m_recipe = std::move(recipe);
	setParent(parent);
}

void TriggerDataStrategy::save(QString data)
{
	if(m_recipe.context == nullptr || m_recipe.device == nullptr) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Invalid arguments, cannot write any data";
		return;
	}

	struct iio_device *trigger = iio_context_find_device(m_recipe.context, data.toStdString().c_str());
	if(trigger == nullptr) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Invalid arguments, no trigger with name" << data << "was found";
	}

	int res;
	if(data == "None") {
		res = iio_device_set_trigger(m_recipe.device, nullptr);

		if(res < 0) {
			qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Cannot clear trigger";
		}
	} else {
		res = iio_device_set_trigger(m_recipe.device, trigger);

		if(res < 0) {
			qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Cannot set trigger" << data;
		}
	}
}

void TriggerDataStrategy::requestData()
{
	QString currentTriggerName, triggerOptions = "None ";

	if(m_recipe.context == nullptr || m_recipe.device == nullptr) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Invalid arguments, cannot read any data";
		return;
	}

	const struct iio_device *currentTrigger;
	ssize_t res = iio_device_get_trigger(m_recipe.device, &currentTrigger);
	if(res < 0) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Cannot read trigger";
		currentTrigger = nullptr;
		currentTriggerName = "None";
	}

	if(currentTrigger != nullptr) {
		currentTriggerName = iio_device_get_name(currentTrigger);
	}

	unsigned int deviceCount = iio_context_get_devices_count(m_recipe.context);
	for(int i = 0; i < deviceCount; ++i) {
		struct iio_device *dev = iio_context_get_device(m_recipe.context, i);
		if(dev == nullptr) {
			qDebug(CAT_TRIGGER_DATA_STRATEGY) << "No device with index" << i << "was found.";
			continue;
		}
		bool isTrigger = iio_device_is_trigger(dev);
		if(isTrigger) {
			QString name = iio_device_get_name(dev);
			if(!name.isEmpty()) {
				triggerOptions += name + " ";
			}
		}
	}

	Q_EMIT sendData(currentTriggerName, triggerOptions);
}

#include "moc_triggerdatastrategy.cpp"
