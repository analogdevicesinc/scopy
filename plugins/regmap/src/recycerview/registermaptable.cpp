#include "recyclerview.hpp"
#include "registermaptable.hpp"

#include <QLabel>
#include "../logging_categories.h"
#include <src/register/registersimplewidget.hpp>
#include <src/register/registersimplewidgetfactory.hpp>

RegisterMapTable::RegisterMapTable(QMap<uint32_t, RegisterModel*> *registerModels)
    :registerModels(registerModels)
{
    registersMap = new QMap<uint32_t, RegisterSimpleWidget*>();

    QList<int> *widgets = new QList<int>();
    foreach(uint32_t index, registerModels->keys()) {
        widgets->push_back((int)index);
    }
    recyclerView = new RecyclerView(widgets);

    QObject::connect(recyclerView, &RecyclerView::requestWidget, this, &RegisterMapTable::generateWidget);
    QObject::connect(this, &RegisterMapTable::widgetGenerated, recyclerView, &RecyclerView::addWidget);

    recyclerView->init();
}

QWidget *RegisterMapTable::getWidget()
{
    return recyclerView;
}

void RegisterMapTable::setFilters(QList<uint32_t> filters)
{
    qDebug(CAT_REGISTER_MAP_TABLE) << "Apply filters ";
    QList<int> *widgets = new QList<int>();
    foreach(uint32_t index, filters) {
        widgets->push_back((int)index);
    }

    recyclerView->hideAll();
    recyclerView->setActiveWidgets(widgets);
    recyclerView->populateMap();
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

void RegisterMapTable::scrollTo(uint32_t index)
{
    if (registerModels->keys().contains(index)) {

        QMap<uint32_t, RegisterModel*>::iterator iterator = registerModels->begin();
        int i = 0;
        while (iterator != registerModels->end() ) {
            if (iterator.key() == index) {
                recyclerView->scrollTo(i);
                setRegisterSelected(iterator.key(), true);
                break;
            }
            i++;
            iterator++;
        }
    }
}

void RegisterMapTable::setRegisterSelected(uint32_t address, bool selected)
{
    registersMap->value(selectedAddress)->setRegisterSelected(false);
    selectedAddress = address;
    registersMap->value(address)->setRegisterSelected(true);
}

void RegisterMapTable::generateWidget(int index)
{
    qDebug(CAT_REGISTER_MAP_TABLE) << "Generate new widget";
    RegisterSimpleWidgetFactory registerSimpleWidgetFactory;
    RegisterSimpleWidget *registerSimpleWidget = registerSimpleWidgetFactory.buildWidget(registerModels->value(index));

    QObject::connect(registerSimpleWidget, &RegisterSimpleWidget::registerSelected, this, &RegisterMapTable::registerSelected);

    registersMap->insert(index, registerSimpleWidget);
    Q_EMIT widgetGenerated(index, registerSimpleWidget);
}
