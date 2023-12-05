#include "attrfactory.h"
#include "savestrategy/timesavestrategy.h"
#include "savestrategy/instantsavestrategy.h"
#include "savestrategy/externalsavestrategy.h"
#include "guistrategy/editableguistrategy.h"
#include "guistrategy/switchguistrategy.h"
#include "datastrategy/attrdatastrategy.h"
#include "datastrategy/triggerdatastrategy.h"
#include "datastrategy/deviceattrdatastrategy.h"
#include "datastrategy/filedemodatastrategy.h"
#include "guistrategy/comboguistrategy.h"
#include "guistrategy/rangeguistrategy.h"
#include <QLoggingCategory>

#define ATTR_BUFFER_SIZE 256
using namespace scopy;
Q_LOGGING_CATEGORY(CAT_ATTRFACTORY, "AttrFactory");

AttrFactory::AttrFactory(QWidget *parent)
	: m_channel(nullptr)
{}

AttrFactory::~AttrFactory() {}

QList<AttrWidget *> AttrFactory::buildAllAttrsForChannel(struct iio_channel *channel)
{
	m_channel = channel;
	QList<AttrWidget *> result;

	QList<QString> channelAttributes;
	ssize_t channelCount = iio_channel_get_attrs_count(m_channel);
	for(int i = 0; i < channelCount; ++i) {
		const char *attrName = iio_channel_get_attr(m_channel, i);
		if(attrName != nullptr) {
			channelAttributes.append(attrName);
		}
	}

	for(const auto &attributeName : channelAttributes) {
		if(attributeName.endsWith("_available")) {
			continue;
		}

		uint32_t hint = AttrData;
		AttributeFactoryRecipe recipe;
		recipe.channel = m_channel;
		recipe.data = attributeName;
		QString availableAttrName = attributeName + "_available";
		if(channelAttributes.contains(availableAttrName)) {
			recipe.dataOptions = availableAttrName;
			char buffer[ATTR_BUFFER_SIZE] = {0};
			ssize_t res = iio_channel_attr_read(m_channel, availableAttrName.toStdString().c_str(), buffer,
							    ATTR_BUFFER_SIZE);
			if(res < 0) {
				qWarning(CAT_ATTRFACTORY) << "Could not read data from" << availableAttrName;
				continue;
			}

			QString readOptions(buffer);
			if(readOptions.startsWith("[")) {
				hint |= RangeUi | TimeSave;
			} else if(readOptions.split(" ", Qt::SkipEmptyParts).size() == 2) {
				hint |= SwitchUi | InstantSave;
			} else {
				hint |= ComboUi | InstantSave;
			}
		} else {
			hint |= EditableUi | InstantSave;
		}

		result.append(this->buildSingle(hint, recipe));
	}

	return result;
}

QList<AttrWidget *> AttrFactory::buildAllAttrsForDevice(struct iio_device *dev)
{
	QList<AttrWidget *> result;

	QList<QString> devAttributes;
	ssize_t devAttrCount = iio_device_get_attrs_count(dev);
	for(int i = 0; i < devAttrCount; ++i) {
		const char *attrName = iio_device_get_attr(dev, i);
		if(attrName != nullptr) {
			devAttributes.append(attrName);
		}
	}

	for(const auto &attributeName : devAttributes) {
		if(attributeName.endsWith("_available")) {
			continue;
		}

		uint32_t hint = DeviceAttrData;
		AttributeFactoryRecipe recipe;
		recipe.device = dev;
		recipe.data = attributeName;
		QString availableAttrName = attributeName + "_available";
		if(devAttributes.contains(availableAttrName)) {
			recipe.dataOptions = availableAttrName;
			char buffer[ATTR_BUFFER_SIZE] = {0};
			ssize_t res = iio_device_attr_read(dev, availableAttrName.toStdString().c_str(), buffer,
							    ATTR_BUFFER_SIZE);
			if(res < 0) {
				qWarning(CAT_ATTRFACTORY) << "Could not read data from" << availableAttrName;
				continue;
			}

			QString readOptions(buffer);
			if(readOptions.startsWith("[")) {
				hint |= RangeUi | TimeSave;
				/*} else if(readOptions.split(" ", Qt::SkipEmptyParts).size() == 2) { // CustomSwitch is broken
					hint |= SwitchUi | InstantSave;*/
			} else {
				hint |= ComboUi | TimeSave;
			}
		} else {
			hint |= EditableUi | TimeSave;
		}

		result.append(this->buildSingle(hint, recipe));
	}

	return result;
}

AttrWidget *AttrFactory::buildSingle(uint32_t hint, AttributeFactoryRecipe recipe)
{
	attr::AttrUiStrategyInterface *uiStrategy = nullptr;
	attr::SaveStrategyInterface *saveStrategy = nullptr;
	attr::DataStrategyInterface *dataStrategy = nullptr;
	AttrWidget *attrWidget = nullptr;

	if(hint & AutoHint) {
		// TODO: implement
	}

	if(hint & TimeSave) {
		saveStrategy = new attr::TimerSaveStrategy(recipe, this);
	} else if(hint & InstantSave) {
		saveStrategy = new attr::InstantSaveStrategy(recipe, this);
	} else if(hint & ExternalSave) {
		saveStrategy = new attr::ExternalTriggerSaveStrategy(recipe, this);
	}

	if(hint & EditableUi) {
		uiStrategy = new attr::EditableGuiStrategy(recipe, this);
	} else if(hint & SwitchUi) {
		uiStrategy = new attr::SwitchAttrUi(recipe, this);
	} else if(hint & ComboUi) {
		uiStrategy = new attr::ComboAttrUi(recipe, this);
	} else if(hint & RangeUi) {
		uiStrategy = new attr::RangeAttrUi(recipe, this);
	}

	if(hint & AttrData) {
		dataStrategy = new attr::AttrDataStrategy(recipe, this);
	} else if(hint & TriggerData) {
		dataStrategy = new attr::TriggerDataStrategy(recipe, this);
	} else if (hint & DeviceAttrData) {
		dataStrategy = new attr::DeviceAttrDataStrategy(recipe, this);
	} else if(hint & FileDemoData) {
		dataStrategy = new attr::FileDemoDataStrategy(recipe, this);
	}

	if(uiStrategy && saveStrategy && dataStrategy) {
		attrWidget = new AttrWidget(uiStrategy, saveStrategy, dataStrategy, this);
		attrWidget->setRecipe(recipe);
	}

	return attrWidget;
}

#include "moc_attrfactory.cpp"
