#include "bitfield/bitfieldsimplewidget.hpp"
#include "registersimplewidget.hpp"
#include <QLabel>
#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qdebug.h>
#include "dynamicWidget.h"
#include <src/utils.hpp>
#include <pluginbase/preferences.h>

RegisterSimpleWidget::RegisterSimpleWidget(QString name, QString address, QString description,
                                           QString notes,int registerWidth, QVector<BitFieldSimpleWidget *> *bitFields, QWidget *parent)
    :bitFields(bitFields),
    registerWidth(registerWidth),
    address(address)
{
    installEventFilter(this);

    scopy::setDynamicProperty(this, "has_frame", true);
    setStyleSheet("::hover {background-color: #4a4a4b; }");

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

    QHBoxLayout *regBaseInfo = new QHBoxLayout();
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignRight);

    QLabel *registerAddressLable = new QLabel(scopy::regmap::Utils::convertToHexa(address.toInt(&ok,16),registerWidth));
    registerAddressLable->setAlignment(Qt::AlignRight);
    rightLayout->addWidget(registerAddressLable);
    value = new QLabel("Not Read");
    rightLayout->addWidget(value);

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setAlignment(Qt::AlignTop);
    QLabel *registerNameLabel = new QLabel(name);
    registerNameLabel->setWordWrap(true);
    leftLayout->addWidget(registerNameLabel);

    regBaseInfo->addLayout(leftLayout);
    regBaseInfo->addLayout(rightLayout);

    regBaseInfoWidget->setLayout(regBaseInfo);
    // to make sure table proportions are good we use a fixed size for this
    regBaseInfoWidget->setFixedWidth(120);
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
        if (col > scopy::regmap::Utils::getBitsPerRow()) {
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
                      + "Address : " + scopy::regmap::Utils::convertToHexa(address.toInt(&ok,16), registerWidth) + "\n"
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
        QString bitFieldValue = scopy::regmap::Utils::convertToHexa(bfVal, bitFields->at(i)->getWidth());
        bitFields->at(i)->updateValue(bitFieldValue);
        regOffset += width;

        bitFields->at(i)->blockSignals(false);
    }
    this->value->setText(scopy::regmap::Utils::convertToHexa(value,registerWidth));
}

void RegisterSimpleWidget::setRegisterSelected(bool selected)
{
    scopy::setDynamicProperty(this,"is_selected",selected);
}

QColor RegisterSimpleWidget::getColor(QString description)
{

    scopy::Preferences *p = scopy::Preferences::GetInstance();
    bool background = p->get("regmap_background_color_by_value").toBool();

    if (background) {
       // get value % 16
        if (m_colors->contains(description)) return m_colors->value(description);

        int red = rand() % 256;
        int blue = rand() % 256;
        int green = rand() % 256;
        m_colors->insert(description, QColor(red,green,blue));
    }
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

