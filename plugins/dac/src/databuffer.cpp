#include "databuffer.h"

#include <QWidget>

using namespace scopy;
using namespace scopy::dac;

DataBuffer::DataBuffer(DataGuiStrategyInterface *guids, DataBufferStrategyInterface *ds, QWidget *parent)
	: QObject(parent)
	, m_dataStrategy(ds)
	, m_guiStrategy(guids)
{
	m_parent = parent;
	if(m_guiStrategy) {
		connect(dynamic_cast<QObject *>(m_guiStrategy), SIGNAL(recipeUpdated(DataBufferRecipe)),
			dynamic_cast<QObject *>(m_dataStrategy), SLOT(recipeUpdated(DataBufferRecipe)));
	}
	connect(dynamic_cast<QObject *>(m_dataStrategy), SIGNAL(loadFinished()), this, SIGNAL(loadFinished()));
	connect(dynamic_cast<QObject *>(m_dataStrategy), SIGNAL(loadFailed()), this, SIGNAL(loadFailed()));
	connect(dynamic_cast<QObject *>(m_dataStrategy), SIGNAL(dataUpdated()), this, SIGNAL(dataUpdated()));
}

DataBuffer::~DataBuffer() {}

DataBufferStrategyInterface *DataBuffer::getDataBufferStrategy() { return m_dataStrategy; }

DataGuiStrategyInterface *DataBuffer::getDataGuiStrategyInterface() { return m_guiStrategy; }

QWidget *DataBuffer::getParent() { return m_parent; }

void DataBuffer::loadData() { m_dataStrategy->loadData(); }
