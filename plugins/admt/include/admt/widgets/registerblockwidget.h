#ifndef REGISTERBLOCKWIDGET_H
#define REGISTERBLOCKWIDGET_H

#include "scopy-admt_export.h"

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>

#include <menusectionwidget.h>
#include <menucollapsesection.h>

namespace scopy::admt {
    class SCOPY_ADMT_EXPORT RegisterBlockWidget : public QWidget
    {
        Q_OBJECT
    public:
        enum ACCESS_PERMISSION{
            READ,
            WRITE,
            READWRITE
        };

        RegisterBlockWidget(QString header, QString description, uint32_t address, uint32_t defaultValue, RegisterBlockWidget::ACCESS_PERMISSION accessPermission, QWidget *parent = nullptr);
    private:
        void addReadButton(QWidget *parent);
        void addWriteButton(QWidget *parent);
        void applyLineEditStyle(QLineEdit *widget);
        void applySpinBoxStyle(QSpinBox *widget);
    };
} // namespace scopy::admt

#endif // REGISTERBLOCKWIDGET_H