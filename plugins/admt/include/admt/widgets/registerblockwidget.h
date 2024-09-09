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

        QPushButton *m_readButton, *m_writeButton;

        RegisterBlockWidget(QString header, QString description, uint32_t address, uint32_t defaultValue, RegisterBlockWidget::ACCESS_PERMISSION accessPermission, QWidget *parent = nullptr);
        QPushButton *readButton();
        QPushButton *writeButton();
        uint32_t getAddress();
        uint32_t getValue();
        void setValue(uint32_t value);
        RegisterBlockWidget::ACCESS_PERMISSION getAccessPermission();
    public Q_SLOTS:
        void onValueChanged(int);
    private:
        uint32_t m_address, m_value;
        RegisterBlockWidget::ACCESS_PERMISSION m_accessPermission;

        QSpinBox *m_spinBox;

        void addReadButton(QWidget *parent);
        void addWriteButton(QWidget *parent);
        void applyLineEditStyle(QLineEdit *widget);
        void applySpinBoxStyle(QSpinBox *widget);
    };
} // namespace scopy::admt

#endif // REGISTERBLOCKWIDGET_H