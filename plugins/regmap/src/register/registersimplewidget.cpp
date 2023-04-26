#include "bitfield/bitfieldsimplewidget.hpp"
#include "registersimplewidget.hpp"
#include <QLabel>
#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qdebug.h>
#include "dynamicWidget.h"
#include <src/utils.hpp>

RegisterSimpleWidget::RegisterSimpleWidget(QString name, QString address, QString description,
                                           QString notes,int registerWidth, QVector<BitFieldSimpleWidget *> *bitFields, QWidget *parent)
    :bitFields(bitFields),
    registerWidth(registerWidth),
    address(address)
{
    installEventFilter(this);

    scopy::setDynamicProperty(this, "has_frame", true);

    setMinimumWidth(10);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    m_colors = new QMap<QString, QColor>();


    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);



    bool ok;

    QFrame *regBaseInfoWidget = new QFrame();
    scopy::setDynamicProperty(regBaseInfoWidget,"has_frame",true);
    QVBoxLayout *regBaseInfo = new QVBoxLayout();
    regBaseInfo->addWidget(new QLabel(Utils::convertToHexa(address.toInt(&ok,16),registerWidth) + " " + name));
    value = new QLabel("Value: Not Read");
    regBaseInfo->addWidget(value);

    regBaseInfoWidget->setLayout(regBaseInfo);
    layout->addWidget(regBaseInfoWidget,1);

    // add bitfield widgets
    QGridLayout *bitFieldsWidgetLayout = new QGridLayout();

    int bits = bitFields->length() - 1;
    int row = 0;
    int col = 0;
    while (bits >= 0 ) {
        int streach = bitFields->at(bits)->getStreach();
        bitFieldsWidgetLayout->addWidget(bitFields->at(bits), row, col, 1, streach);
        col += streach;
        if (col > 7) {
            row++;
            col = 0;
        }
        --bits;
    }

    for (int i = 0; i < bitFieldsWidgetLayout->columnCount(); i++){
        bitFieldsWidgetLayout->setColumnStretch(i,1);
    }

    layout->addLayout(bitFieldsWidgetLayout,8);

    QString toolTip = "Name : " + name + "\n"
                      + "Address : " + Utils::convertToHexa(address.toInt(&ok,16), registerWidth) + "\n"
                      + "Description : " + description + "\n"
                      + "Notes : " + notes + "\n" ;

    setToolTip(toolTip);
}

RegisterSimpleWidget::~RegisterSimpleWidget()
{
    delete value;
}

void RegisterSimpleWidget::valueUpdated(uint32_t value)
{
    int regOffset = 0;
    for (int i = 0; i < bitFields->length(); ++i) {
        bitFields->at(i)->blockSignals(true);

        int width = bitFields->at(i)->getWidth();
        int bfVal = ( ((1 << (regOffset + width) ) - 1 ) & value) >> regOffset;
        QString bitFieldValue = Utils::convertToHexa(bfVal, bitFields->at(i)->getWidth());
        bitFields->at(i)->updateValue(bitFieldValue);
        regOffset += width;

        bitFields->at(i)->blockSignals(false);
    }
    this->value->setText(Utils::convertToHexa(value,registerWidth));
}

QColor RegisterSimpleWidget::getColor(QString description)
{

    if (m_colors->contains(description)) return m_colors->value(description);

    int red = rand() % 256;
    int blue = rand() % 256;
    int green = rand() % 256;
    m_colors->insert(description, QColor(red,green,blue));
    return m_colors->value(description);
}

bool RegisterSimpleWidget::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        bool ok;
        Q_EMIT registerSelected(address.toInt(&ok,16));
    }
    return QWidget::eventFilter(object,event);
}

