#include "savestrategy/externalsavestrategy.h"

#include <gui/stylehelper.h>
#include <QLoggingCategory>

using namespace scopy::attr;

Q_LOGGING_CATEGORY(CAT_ATTR_SAVE_STRATEGY, "AttrSaveStrategy")

ExternalSaveStrategy::ExternalSaveStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent)
	: m_recipe(recipe)
	, m_ui(new QPushButton("Write"))
	, m_dataReceived(false)
{
	setParent(parent);
	StyleHelper::SmallBlueButton(m_ui, "SaveButton" + m_recipe.data);
	connect(m_ui, &QPushButton::clicked, this, &ExternalSaveStrategy::writeData);
}

ExternalSaveStrategy::~ExternalSaveStrategy() { m_ui->deleteLater(); }

QWidget *ExternalSaveStrategy::ui() { return m_ui; }

bool ExternalSaveStrategy::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "") {
		return true;
	}
	return false;
}

void ExternalSaveStrategy::receiveData(QString data)
{
	qDebug(CAT_ATTR_SAVE_STRATEGY) << "received" << data;
	m_data = data;
	m_dataReceived = true;
	Q_EMIT receivedData();
}

void ExternalSaveStrategy::writeData()
{
	if(m_dataReceived) {
		Q_EMIT saveData(m_data);
	}
	qCritical(CAT_ATTR_SAVE_STRATEGY) << "Writing from external" << m_data << "to" << m_recipe.data;
}

#include "moc_externalsavestrategy.cpp"
