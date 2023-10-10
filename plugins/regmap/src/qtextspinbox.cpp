#include "qtextspinbox.hpp"

QTextSpinBox::QTextSpinBox(QWidget *parent)
	: QSpinBox(parent)
{}

void QTextSpinBox::setValues(QList<QString> *acceptedValues)
{
	this->acceptedValues = acceptedValues;
	setRange(1, acceptedValues->size());
}

QString QTextSpinBox::textFromValue(int value) const { return acceptedValues->value(value); }

int QTextSpinBox::valueFromText(const QString &text) const { return acceptedValues->indexOf(text); }

QValidator::State QTextSpinBox::validate(QString &text, int &pos) const
{
	if(acceptedValues->contains(text)) {
		return QValidator::Acceptable;
	}

	if(!text.isNull()) {
		return QValidator::Intermediate;
	}

	return QValidator::Invalid;
}
