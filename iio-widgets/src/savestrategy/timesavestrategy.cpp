#include "savestrategy/timesavestrategy.h"
#include <gui/stylehelper.h>
#include <QLoggingCategory>
#include <utility>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_TIME_DATA_STRATEGY, "TimeSaveStrategy")

TimerSaveStrategy::TimerSaveStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: QWidget(parent)
	, m_recipe(recipe)
	, m_progressBar(new SmallProgressBar(nullptr))
{
	connect(m_progressBar, &SmallProgressBar::progressFinished, this, &TimerSaveStrategy::writeData);
}

QWidget *TimerSaveStrategy::ui() { return m_progressBar; }

bool TimerSaveStrategy::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "") {
		return true;
	}
	return false;
}

void TimerSaveStrategy::receiveData(QString data)
{
	qDebug(CAT_TIME_DATA_STRATEGY) << "Saving data" << data << "and restarting timer";
	m_lastData = data;
	m_progressBar->startProgress();
}

void TimerSaveStrategy::writeData()
{
	Q_EMIT saveData(m_lastData);
	qCritical(CAT_TIME_DATA_STRATEGY) << "Writing late" << m_lastData << "to" << m_recipe.data;
}

#include "moc_timesavestrategy.cpp"
