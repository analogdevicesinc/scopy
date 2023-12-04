#include "guistrategy/editableguistrategy.h"

using namespace scopy::attr;

EditableGuiStrategy::EditableGuiStrategy(AttributeFactoryRecipe recipe, QObject *parent)
	: m_ui(new QWidget(nullptr))
	, m_lineEdit(new QLineEdit(m_ui))
{
	setParent(parent);
	m_recipe = std::move(recipe);

	m_ui->setLayout(new QVBoxLayout(m_ui));
	m_ui->layout()->setContentsMargins(0, 0, 0, 0);
	m_ui->layout()->addWidget(m_lineEdit);

	connect(m_lineEdit, &QLineEdit::editingFinished, this, [this]() { Q_EMIT emitData(m_lineEdit->text()); });

	Q_EMIT requestData();
}

EditableGuiStrategy::~EditableGuiStrategy() { m_ui->deleteLater(); }

QWidget *EditableGuiStrategy::ui() { return m_ui; }

bool EditableGuiStrategy::isValid()
{
	if(m_recipe.data != "" && m_recipe.channel != nullptr) {
		return true;
	}
	return false;
}

void EditableGuiStrategy::receiveData(QString currentData, QString optionalData)
{
	QSignalBlocker blocker(m_lineEdit);
	m_lineEdit->setText(currentData);
}

#include "moc_editableguistrategy.cpp"
