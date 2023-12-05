#include "guistrategy/comboguistrategy.h"
#include <QLoggingCategory>
#include <utility>

using namespace scopy::attr;

ComboAttrUi::ComboAttrUi(AttributeFactoryRecipe recipe, QObject *parent)
	: m_ui(new QWidget(nullptr))
{
	setParent(parent);
	m_recipe = recipe;
	m_ui->setLayout(new QVBoxLayout(m_ui));
	m_ui->layout()->setContentsMargins(0, 0, 0, 0);

	m_comboWidget = new MenuCombo(m_recipe.data, m_ui);
	m_ui->layout()->addWidget(m_comboWidget);
	Q_EMIT requestData();

	connect(m_comboWidget->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		[this](int index) {
			QString currentData = m_comboWidget->combo()->currentText();
			Q_EMIT emitData(currentData);
		});
}

ComboAttrUi::~ComboAttrUi() { m_ui->deleteLater(); }

QWidget *ComboAttrUi::ui() { return m_ui; }

bool ComboAttrUi::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "" && m_recipe.dataOptions != "") {
		return true;
	}
	return false;
}

void ComboAttrUi::receiveData(QString currentData, QString optionalData)
{
	auto combo = m_comboWidget->combo();
	QSignalBlocker blocker(combo);
	QStringList optionsList = QString(optionalData).split(" ", Qt::SkipEmptyParts);
	for(const QString &item : optionsList) {
		combo->addItem(item);
	}

	combo->setCurrentText(currentData);
}

#include "moc_comboguistrategy.cpp"
