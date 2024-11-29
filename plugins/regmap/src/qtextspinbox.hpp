#ifndef QTEXTSPINBOX_HPP
#define QTEXTSPINBOX_HPP

#include <QSpinBox>
#include <QObject>
#include <QWidget>
#include "scopy-regmapplugin_export.h"

class SCOPY_REGMAPPLUGIN_EXPORT QTextSpinBox : public QSpinBox
{
public:
    explicit QTextSpinBox(QWidget *parent = 0);

    void setValues(QList<QString> *acceptedValues);

private:
    QString textFromValue(int value) const;
    int valueFromText(const QString &text) const;
    QValidator::State validate(QString &text, int &pos) const;
    QList<QString> *acceptedValues;
};

#endif // QTEXTSPINBOX_HPP
