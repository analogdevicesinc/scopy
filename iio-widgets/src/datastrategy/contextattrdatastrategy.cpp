#include "contextattrdatastrategy.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_CONTEXT_ATTR_DATA_STRATEGY, "ContextAttrDataStrategy")
using namespace scopy;

ContextAttrDataStrategy::ContextAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: QWidget(parent)
{
	m_recipe = recipe;
	m_optionalData = "";
}

QString ContextAttrDataStrategy::data() { return m_data; }

QString ContextAttrDataStrategy::optionalData() { return m_optionalData; }

void ContextAttrDataStrategy::save(QString data)
{
	qInfo(CAT_CONTEXT_ATTR_DATA_STRATEGY) << "ContextAttrDataStrategy::save called, but the attr is read only";
	requestData();
}

void ContextAttrDataStrategy::requestData()
{
	if(m_recipe.context == nullptr || m_recipe.data == "") {
		qWarning(CAT_CONTEXT_ATTR_DATA_STRATEGY) << "Invalid arguments, cannot read any data";
		return;
	}

	const char *value = iio_context_get_attr_value(m_recipe.context, m_recipe.data.toStdString().c_str());

	if(value == nullptr) {
		qWarning(CAT_CONTEXT_ATTR_DATA_STRATEGY) << "Could not read value from context attr" << m_recipe.data;
		return;
	}

	m_data = value;
	Q_EMIT sendData(m_data, "");
}

#include "moc_contextattrdatastrategy.cpp"
