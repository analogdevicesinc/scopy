#include "textspinbox.hpp"

TextSpinBox::TextSpinBox(QWidget *parent)
	: QSpinBox(parent)
{}

void TextSpinBox::setValues(QList<QString> *acceptedValues)
{
	this->acceptedValues = acceptedValues;
	setRange(1, acceptedValues->size());
}

QString TextSpinBox::textFromValue(int value) const { return acceptedValues->value(value); }

int TextSpinBox::valueFromText(const QString &text) const { return acceptedValues->indexOf(text); }

QValidator::State TextSpinBox::validate(QString &text, int &pos) const
{
	if(acceptedValues->contains(text)) {
		return QValidator::Acceptable;
	}

	if(!text.isNull()) {
		return QValidator::Intermediate;
	}

	return QValidator::Invalid;
}
