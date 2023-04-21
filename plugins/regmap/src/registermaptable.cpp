#include "registermaptable.hpp"
#include "verticalscrollarea.hpp"
#include <QLineEdit>
#include <qboxlayout.h>
#include <qboxlayout.h>
#include <qpushbutton.h>
#include "register/registersimplewidgetfactory.hpp"
#include "register/registermodel.hpp"
#include "register/bitfield/bitfieldmodel.hpp"
#include "register/registersimplewidget.hpp"
#include <QScrollBar>
#include <QDebug>
#include <QGridLayout>
#include <QtConcurrent/QtConcurrent>
#include "logging_categories.h"

RegisterMapTable::RegisterMapTable(QMap<uint32_t, RegisterModel *> *registerModels, QWidget *parent)
    : registerModels(registerModels),
      QWidget{parent}
{
    m_layout = new QVBoxLayout();
    setLayout(m_layout);
    setStyleSheet("border: 1px solid black");
    m_scrollArea = new VerticalScrollArea();
    m_scrollArea->setTabletTracking(true);

    QObject::connect(m_scrollArea->verticalScrollBar(), &QAbstractSlider::valueChanged, this, [=](int value){
        if (value == m_scrollArea->verticalScrollBar()->minimum()) {
            scrollUp();
        }
        if (value == m_scrollArea->verticalScrollBar()->maximum()){
            scrollDown();
        }
    });

    m_registerTableLayout = new QGridLayout();
    m_registerTable = new QWidget();
    m_registerTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    m_registerTable->setLayout(m_registerTableLayout);

    registersMap = new QMap<uint32_t, RegisterSimpleWidget*>();

    m_scrollArea->setWidget(m_registerTable);
    m_layout->addWidget(m_scrollArea);

    foreach (uint32_t key, registerModels->keys()) {
        activeWidgets.push_back(key);
    }
    populateMap();
}

RegisterMapTable::~RegisterMapTable()
{
    delete m_layout;
    delete m_scrollArea;

    delete registersMap;
}

void RegisterMapTable::valueUpdated(uint32_t address, uint32_t value)
{
    qDebug(CAT_REGISTER_MAP_TABLE) << "Update value for register at address " << address;
    if (registersMap->contains(address)) {
        registersMap->value(address)->valueUpdated(value);
    } else {
        qDebug(CAT_REGISTER_MAP_TABLE) << "No register was found for address " << address;
    }
}

void RegisterMapTable::setFilters(QList<uint32_t> filters)
{
    qDebug(CAT_REGISTER_MAP_TABLE) << "Apply filters ";
    hideAll();
    activeWidgets = filters;
    populateMap();
}

void RegisterMapTable::hideAll()
{
    QMap<uint32_t, RegisterSimpleWidget*>::iterator mapIterator;
    for (mapIterator = registersMap->begin(); mapIterator != registersMap->end(); ++mapIterator) {
        mapIterator.value()->hide();
    }
}

void RegisterMapTable::showAll()
{
    QMap<uint32_t, RegisterSimpleWidget*>::iterator mapIterator;
    for (mapIterator = registersMap->begin(); mapIterator != registersMap->end(); ++mapIterator) {
        mapIterator.value()->show();
    }
}

void RegisterMapTable::setMaxVisibleRows(int newMaxVisibleRows)
{
    m_maxVisibleRows = newMaxVisibleRows;
}

void RegisterMapTable::scrollDown()
{
    qDebug(CAT_REGISTER_MAP_TABLE) << "Scroll reached bottom";
    if (activeWidgetBottom != activeWidgets.end()) {
        registersMap->value(*activeWidgetTop)->hide();
        if (registersMap->contains(*activeWidgetBottom)) {
            registersMap->value(*activeWidgetBottom)->show();
        }
        else {
            generateWidget(registerModels->value(*activeWidgetBottom));
        }

        // TODO optimize
        m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->maximum() - registersMap->value(*activeWidgetBottom)->sizeHint().height());
        activeWidgetTop++;
        activeWidgetBottom++;
    }
}

void RegisterMapTable::scrollUp()
{
    qDebug(CAT_REGISTER_MAP_TABLE) << "Scroll reached top";
    if (activeWidgetTop != activeWidgets.begin()) {
        activeWidgetTop--;
        activeWidgetBottom--;
        registersMap->value(*activeWidgetBottom)->hide();
        if (registersMap->contains(*activeWidgetTop)) {
            registersMap->value(*activeWidgetTop)->show();
        }
        else {
            generateWidget(registerModels->value(*activeWidgetTop));
        }
        // TODO optimize
        m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->minimum() + registersMap->value(*activeWidgetTop)->sizeHint().height());
    }
}

void RegisterMapTable::populateMap()
{
    QList<uint32_t>::iterator mapIterator = activeWidgets.begin();
    int i = 0;
    while ( i < m_maxVisibleRows && mapIterator != activeWidgets.end()) {
        if (registersMap->contains(*mapIterator)) {
            registersMap->value(*mapIterator)->show();
        }
        else {
            generateWidget(registerModels->value(*mapIterator));
        }
        ++mapIterator;
        ++i;
    }
    activeWidgetBottom = mapIterator;
    activeWidgetTop = activeWidgets.begin();

}

void RegisterMapTable::generateWidget(RegisterModel *model)
{
    qDebug(CAT_REGISTER_MAP_TABLE) << "Generate new widget";
    RegisterSimpleWidgetFactory registerSimpleWidgetFactory;
    RegisterSimpleWidget *registerSimpleWidget = registerSimpleWidgetFactory.buildWidget(model);
    QObject::connect(registerSimpleWidget, &RegisterSimpleWidget::registerSelected, this, &RegisterMapTable::registerSelected);
    registersMap->insert(model->getAddress(), registerSimpleWidget);
    m_registerTableLayout->addWidget(registerSimpleWidget,model->getAddress(),0);
}
