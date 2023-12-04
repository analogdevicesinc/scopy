#include "attrwidget.h"

using namespace scopy;

AttrWidget::AttrWidget(attr::AttrUiStrategyInterface *uiStrategy, attr::SaveStrategyInterface *saveStrategy,
		       attr::DataStrategyInterface *dataStrategy, QWidget *parent)
	: QWidget(parent)
	, m_uiStrategy(uiStrategy)
	, m_saveStrategy(saveStrategy)
	, m_dataStrategy(dataStrategy)
{
	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	QWidget *ui = uiStrategy->ui();
	if(ui) {
		layout()->addWidget(ui);
	}

	QWidget *saveUi = saveStrategy->ui();
	if(saveUi) {
		layout()->addWidget(saveUi);
	}

	connect(m_uiStrategy, &attr::AttrUiStrategyInterface::emitData, m_saveStrategy,
		&attr::SaveStrategyInterface::receiveData);
	connect(m_saveStrategy, &attr::SaveStrategyInterface::saveData, m_dataStrategy,
		&attr::DataStrategyInterface::save);

	connect(m_uiStrategy, &attr::AttrUiStrategyInterface::requestData, m_dataStrategy,
		&attr::DataStrategyInterface::requestData);
	connect(m_dataStrategy, &attr::DataStrategyInterface::sendData, m_uiStrategy,
		&attr::AttrUiStrategyInterface::receiveData);

	m_dataStrategy->requestData();
}

attr::SaveStrategyInterface *AttrWidget::getSaveStrategy() { return m_saveStrategy; }

attr::AttrUiStrategyInterface *AttrWidget::getUiStrategy() { return m_uiStrategy; }

attr::DataStrategyInterface *AttrWidget::getDataStrategy() { return m_dataStrategy; }

AttributeFactoryRecipe AttrWidget::getRecipe() { return m_recipe; }

void AttrWidget::setRecipe(AttributeFactoryRecipe recipe) { m_recipe = recipe; }

#include "moc_attrwidget.cpp"
