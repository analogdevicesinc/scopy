#include "attrwidget.h"

using namespace scopy;

AttrWidget::AttrWidget(QString title, attr::AttrUiStrategyInterface *uiStrategy,
		       attr::SaveStrategyInterface *saveStrategy, attr::DataStrategyInterface *dataStrategy,
		       QWidget *parent)
	: QWidget(parent)
	, m_uiStrategy(uiStrategy)
	, m_saveStrategy(saveStrategy)
	, m_dataStrategy(dataStrategy)
	, m_menuSectionWidget(new MenuSectionWidget(this))
	, m_collapseSection(new MenuCollapseSection(title.toUpper(), MenuCollapseSection::MHCW_NONE, this))
{
	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	layout()->addWidget(m_menuSectionWidget);
	m_menuSectionWidget->contentLayout()->addWidget(m_collapseSection);
	QWidget *ui = m_uiStrategy->ui();
	if(ui) {
		m_collapseSection->contentLayout()->addWidget(ui);
	}

	QWidget *saveUi = m_saveStrategy->ui();
	if(saveUi) {
		m_collapseSection->contentLayout()->addWidget(saveUi);
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


