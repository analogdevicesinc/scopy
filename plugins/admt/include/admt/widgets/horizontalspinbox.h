#ifndef HORIZONTALSPINBOX_H
#define HORIZONTALSPINBOX_H

#include "scopy-admt_export.h"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

namespace scopy::admt {
    class SCOPY_ADMT_EXPORT HorizontalSpinBox : public QWidget
    {
        Q_OBJECT
    public:
        HorizontalSpinBox(QString header = "", double initialValue = 0.0, QString unit = "", QWidget *parent = nullptr);
        QLineEdit *lineEdit();
        void setEnabled(double value);
    public Q_SLOTS:
        void setValue(double);
    protected Q_SLOTS:
        void onMinusButtonPressed();
        void onPlusButtonPressed();
        void onLineEditTextEdited();
    private:
        double m_value = 0;
        QString m_unit = "";
        QLineEdit *m_lineEdit;
        QPushButton *m_minusButton, *m_plusButton;
        QLabel *m_unitLabel;
        void applyLineEditStyle(QLineEdit *widget);
        void applyPushButtonStyle(QPushButton *widget, int topLeftBorderRadius = 0, int topRightBorderRadius = 0, int bottomLeftBorderRadius = 0, int bottomRightBorderRadius = 0);
        void applyUnitLabelStyle(QLabel *widget, bool isEnabled = true);
    };
} // namespace scopy::admt

#endif // HORIZONTALSPINBOX_H