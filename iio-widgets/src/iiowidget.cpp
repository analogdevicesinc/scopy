#include "iiowidget.h"

using namespace scopy;

IIOWidget::IIOWidget(attr::AttrUiStrategyInterface *uiStrategy, attr::SaveStrategyInterface *saveStrategy,
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

	connect(m_saveStrategy, &attr::SaveStrategyInterface::saveData, this, [this](QString data) {
		Q_EMIT currentStateChanged(State::Busy, "Operation in progress.");
		m_dataStrategy->save(data);
	});

	connect(m_dataStrategy, &attr::DataStrategyInterface::emitStatus, this, [this](int status) {
		if(status < 0) {
			Q_EMIT currentStateChanged(State::Error, "Error: " + QString(strerror(-status)));
		} else {
			Q_EMIT currentStateChanged(State::Correct, "Operation finished successfully.");
		}
	});

	connect(m_uiStrategy, &attr::AttrUiStrategyInterface::requestData, m_dataStrategy,
		&attr::DataStrategyInterface::requestData);
	connect(m_dataStrategy, &attr::DataStrategyInterface::sendData, m_uiStrategy,
		&attr::AttrUiStrategyInterface::receiveData);

	m_dataStrategy->requestData();
}

attr::SaveStrategyInterface *IIOWidget::getSaveStrategy() { return m_saveStrategy; }

attr::AttrUiStrategyInterface *IIOWidget::getUiStrategy() { return m_uiStrategy; }

attr::DataStrategyInterface *IIOWidget::getDataStrategy() { return m_dataStrategy; }

IIOWidgetFactoryRecipe IIOWidget::getRecipe() { return m_recipe; }

void IIOWidget::setRecipe(IIOWidgetFactoryRecipe recipe) { m_recipe = recipe; }

#include "moc_iiowidget.cpp"
