#include "triggermenusection.h"
#include "stylehelper.h"

using namespace scopy;

TriggerMenuSection::TriggerMenuSection(QWidget *parent)
	: QWidget(parent)
{
	m_menuSectionWidget = new MenuSectionWidget(this);
	m_menuComboWidget = new MenuComboWidget("Triggers", m_menuSectionWidget);
	setupUi();
	m_menuComboWidget->combo()->addItem("None");
	connect(m_menuComboWidget->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		[this](int index) {
			QString triggerName = m_menuComboWidget->combo()->currentText();
			Q_EMIT selectedTrigger(triggerName);
		});
}

void TriggerMenuSection::selectTrigger(const QString &trigger)
{
	m_menuComboWidget->blockSignals(true); // do not emit signals
	m_menuComboWidget->combo()->setCurrentText(trigger);
	m_menuComboWidget->blockSignals(false);
}

void TriggerMenuSection::addTrigger(const QString &trigger) { m_menuComboWidget->combo()->addItem(trigger); }

void TriggerMenuSection::setupUi()
{
	setLayout(new QVBoxLayout(this));
	layout()->setSpacing(9);
	layout()->setContentsMargins(0, 0, 0, 0);
	m_menuSectionWidget->contentLayout()->setSpacing(9);
	m_menuSectionWidget->contentLayout()->setContentsMargins(0, 0, 0, 0);
	StyleHelper::MenuComboWidget(m_menuComboWidget, "TriggerMenuCombo");

	m_menuSectionWidget->contentLayout()->addWidget(m_menuComboWidget);
	layout()->addWidget(m_menuSectionWidget);
	layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

#include "moc_triggermenusection.cpp"
