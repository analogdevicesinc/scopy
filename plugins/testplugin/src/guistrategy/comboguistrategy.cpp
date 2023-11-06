#include "guistrategy/comboguistrategy.h"
#include <QLoggingCategory>
#include <utility>

using namespace scopy::attr;

ComboAttrUi::ComboAttrUi(AttributeFactoryRecipe recipe, QObject *parent)
	: m_ui(new QWidget(nullptr))
{
	setParent(parent);
	m_recipe = std::move(recipe);
	m_ui->setLayout(new QVBoxLayout(m_ui));
	m_ui->layout()->setContentsMargins(0, 0, 0, 0);

	m_menuComboWidget = new MenuComboWidget(m_recipe.data, m_ui);
	m_menuComboWidget->layout()->setContentsMargins(0, 0, 0, 0);
	m_menuComboWidget->layout()->setMargin(0);
	m_ui->layout()->addWidget(m_menuComboWidget);
	Q_EMIT requestData();

	connect(m_menuComboWidget->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		[this](int index) {
			QString currentData = m_menuComboWidget->combo()->currentText();
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
	QStringList optionsList = QString(optionalData).split(" ", Qt::SkipEmptyParts);
	for(const QString &item : optionsList) {
		m_menuComboWidget->combo()->addItem(item);
	}

	m_menuComboWidget->combo()->setCurrentText(currentData);
}

#include "moc_comboguistrategy.cpp"
