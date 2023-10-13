#ifndef TEXTSPINBOX_H
#define TEXTSPINBOX_H

#include "scopy-regmapplugin_export.h"

#include <QObject>
#include <QSpinBox>
#include <QWidget>

class SCOPY_REGMAPPLUGIN_EXPORT TextSpinBox : public QSpinBox
{
public:
	explicit TextSpinBox(QWidget *parent = 0);

	void setValues(QList<QString> *acceptedValues);

private:
	QString textFromValue(int value) const;
	int valueFromText(const QString &text) const;
	QValidator::State validate(QString &text, int &pos) const;
	QList<QString> *acceptedValues;
};

#endif // TEXTSPINBOX_H
