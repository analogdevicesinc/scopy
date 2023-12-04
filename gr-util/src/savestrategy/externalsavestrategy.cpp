#include "savestrategy/externalsavestrategy.h"

#include <gui/stylehelper.h>
#include <QLoggingCategory>
#include <utility>

using namespace scopy::attr;

Q_LOGGING_CATEGORY(CAT_ATTR_SAVE_STRATEGY, "AttrSaveStrategy")

ExternalTriggerSaveStrategy::ExternalTriggerSaveStrategy(AttributeFactoryRecipe recipe, QObject *parent)
	: m_recipe(std::move(recipe))
	, m_ui(new QPushButton("Write"))
	, m_dataReceived(false)
{
	setParent(parent);
	StyleHelper::SmallBlueButton(m_ui, "SaveButton" + m_recipe.data);
	connect(m_ui, &QPushButton::clicked, this, &ExternalTriggerSaveStrategy::writeData);
}

ExternalTriggerSaveStrategy::~ExternalTriggerSaveStrategy() { m_ui->deleteLater(); }

QWidget *ExternalTriggerSaveStrategy::ui() { return m_ui; }

bool ExternalTriggerSaveStrategy::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "") {
		return true;
	}
	return false;
}

void ExternalTriggerSaveStrategy::receiveData(QString data)
{
	qDebug(CAT_ATTR_SAVE_STRATEGY) << "received" << data;
	m_data = data;
	m_dataReceived = true;
}

void ExternalTriggerSaveStrategy::writeData()
{
	if(m_dataReceived) {
		Q_EMIT saveData(m_data);
	}
	qCritical(CAT_ATTR_SAVE_STRATEGY) << "Writing from external" << m_data << "to" << m_recipe.data;
}

#include "moc_externalsavestrategy.cpp"
