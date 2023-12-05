#include "guistrategy/rangeguistrategy.h"

using namespace scopy::attr;

Q_LOGGING_CATEGORY(CAT_ATTR_GUI_STRATEGY, "AttrGuiStrategy")

RangeAttrUi::RangeAttrUi(AttributeFactoryRecipe recipe, QObject *parent)
	: m_ui(new QWidget(nullptr))
{
	setParent(parent);
	m_ui->setLayout(new QVBoxLayout(m_ui));

	// FIXME: this does not look right when uninitialized, also crashes...
	m_positionSpinButton = new PositionSpinButton({{m_recipe.dataOptions, 1}}, m_recipe.data);
	StyleHelper::MenuSpinBox(m_positionSpinButton, "RangeSpinButton");
	m_ui->layout()->addWidget(m_positionSpinButton);
	Q_EMIT requestData();

	connect(m_positionSpinButton, &PositionSpinButton::valueChanged, this,
		[this](double value) { Q_EMIT emitData(QString::number(value)); });
}

RangeAttrUi::~RangeAttrUi() { m_ui->deleteLater(); }

QWidget *RangeAttrUi::ui() { return m_ui; }

bool RangeAttrUi::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "" && m_recipe.dataOptions != "") {
		return true;
	}
	return false;
}

void RangeAttrUi::receiveData(QString currentData, QString optionalData)
{
	QSignalBlocker blocker(m_positionSpinButton);
	QString availableAttributeValue = QString(optionalData).mid(1, QString(optionalData).size() - 2);
	QStringList optionsList = availableAttributeValue.split(" ", Qt::SkipEmptyParts);
	bool ok = true, finalOk = true;
	double min = optionsList[0].toDouble(&ok);
	finalOk &= ok;
	double step = optionsList[1].toDouble(&ok);
	finalOk &= ok;
	double max = optionsList[2].toDouble(&ok);
	finalOk &= ok;
	double currentNum = QString(currentData).toDouble(&ok);
	finalOk &= ok;
	if(!finalOk) {
		qWarning(CAT_ATTR_GUI_STRATEGY)
			<< "Could not parse the values from" << availableAttributeValue << "as double ";
	}
	m_positionSpinButton->setMinValue(min);
	m_positionSpinButton->setMaxValue(max);
	m_positionSpinButton->setStep(step);
	m_positionSpinButton->setValue(currentNum);
}
#include "moc_rangeguistrategy.cpp"
