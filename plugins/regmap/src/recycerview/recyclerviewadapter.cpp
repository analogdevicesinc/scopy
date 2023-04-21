#include "recyclerview.hpp"
#include "recyclerviewadapter.hpp"

#include <QLabel>
#include "../logging_categories.h"
#include <src/register/registersimplewidget.hpp>
#include <src/register/registersimplewidgetfactory.hpp>

RecyclerViewAdapter::RecyclerViewAdapter(QMap<uint32_t, RegisterModel*> *registerModels)
    :registerModels(registerModels)
{
    registersMap = new QMap<uint32_t, RegisterSimpleWidget*>();

    QList<int> *widgets = new QList<int>();
    foreach(uint32_t index, registerModels->keys()) {
        widgets->push_back((int)index);
    }
    recyclerView = new RecyclerView(widgets);

    QObject::connect(recyclerView, &RecyclerView::requestWidget, this, &RecyclerViewAdapter::generateWidget);
    QObject::connect(this, &RecyclerViewAdapter::widgetGenerated, recyclerView, &RecyclerView::addWidget);

    recyclerView->init();
}

QWidget *RecyclerViewAdapter::getWidget()
{
    return recyclerView;
}

void RecyclerViewAdapter::setFilters(QList<uint32_t> filters)
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

void RecyclerViewAdapter::valueUpdated(uint32_t address, uint32_t value)
{
    qDebug(CAT_REGISTER_MAP_TABLE) << "Update value for register at address " << address;
    if (registersMap->contains(address)) {
        registersMap->value(address)->valueUpdated(value);
    } else {
        qDebug(CAT_REGISTER_MAP_TABLE) << "No register was found for address " << address;
    }
}

void RecyclerViewAdapter::generateWidget(int index)
{
    qDebug(CAT_REGISTER_MAP_TABLE) << "Generate new widget";
    RegisterSimpleWidgetFactory registerSimpleWidgetFactory;
    RegisterSimpleWidget *registerSimpleWidget = registerSimpleWidgetFactory.buildWidget(registerModels->value(index));

    QObject::connect(registerSimpleWidget, &RegisterSimpleWidget::registerSelected, this, &RecyclerViewAdapter::registerSelected);


   registersMap->insert(index, registerSimpleWidget);
  //  m_registerTableLayout->addWidget(registerSimpleWidget,index,0);

    Q_EMIT widgetGenerated(index, registerSimpleWidget);
}
