#include "recyclerview.hpp"
#include <QGridLayout>
#include <QSlider>
#include <QScrollBar>
#include <qlabel.h>
#include <QDebug>
#include <QEvent>
#include <QScrollEvent>

RecyclerView::RecyclerView(QList<int> *widgets,QWidget *parent)
    : widgets(widgets),
    QWidget{parent}
{
    this->installEventFilter(this);
    this->setLayout(new QHBoxLayout());

    bitFieldsWidgetLayout = new QGridLayout();
    setStyleSheet("border: 1px solid black");
    QWidget *bitFieldsWidget = new QWidget();
    bitFieldsWidget->setLayout(bitFieldsWidgetLayout);

    this->layout()->addWidget(bitFieldsWidget);

    slider = new QSlider();
    slider->setInvertedAppearance(true);
    slider->setInvertedControls(true);
    slider->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::MinimumExpanding);

    this->layout()->addWidget(slider);

}

RecyclerView::~RecyclerView()
{
    delete slider;
    delete bitFieldsWidgetLayout;
    delete widgetMap;
}

void RecyclerView::init()
{
    widgetMap = new QMap<int, QWidget*>();

    populateMap();

    m_scrollBarCurrentValue = slider->value();

    QObject::connect(slider, &QAbstractSlider::valueChanged, this, [=](int value){

        if(m_scrollBarCurrentValue < value) {
            int diff = value - *activeWidgetTop;
            while (diff > 0 ) {
                scrollDown();
                diff--;
            }

        } else {
            int diff = *activeWidgetTop - value;
            while (diff > 0 ) {
                scrollUp();
                diff--;
            }
        }
        m_scrollBarCurrentValue = value;
    });

}

QMap<int, QWidget*> *RecyclerView::getWidgetsMap() const
{
    return widgetMap;
}

void RecyclerView::setWidgetMap(QMap<int, QWidget*> *newWidgets)
{
    widgetMap = newWidgets;
}

void RecyclerView::addWidget(int index, QWidget *widget)
{
    widgetMap->insert(index,widget);
    bitFieldsWidgetLayout->addWidget(widget, index, 0);
}

QWidget *RecyclerView::getWidgetAtIndex(int index)
{
    if (widgetMap->value(index) == bitFieldsWidgetLayout->itemAtPosition(index,0)->widget()){
        qDebug()<<"TRUE";
    }
    return widgetMap->value(index);
}

void RecyclerView::hideAll()
{
    QMap<int, QWidget*>::iterator mapIterator;
    for (mapIterator = widgetMap->begin(); mapIterator != widgetMap->end(); ++mapIterator) {
        mapIterator.value()->hide();
    }
}

void RecyclerView::showAll()
{
    QMap<int, QWidget*>::iterator mapIterator;
    for (mapIterator = widgetMap->begin(); mapIterator != widgetMap->end(); ++mapIterator) {
        mapIterator.value()->show();
    }
}

void RecyclerView::setActiveWidgets(QList<int> *widgets)
{
    this->widgets = widgets;
}

void RecyclerView::scrollDown()
{
    if (activeWidgetBottom != widgets->end()) {
        widgetMap->value(*activeWidgetTop)->hide();
        if (widgetMap->contains(*activeWidgetBottom)) {
            widgetMap->value(*activeWidgetBottom)->show();
        }
        else {
            Q_EMIT requestWidget(*activeWidgetBottom);
        }

        activeWidgetTop++;
        activeWidgetBottom++;
    }
}

void RecyclerView::scrollUp()
{
    if (activeWidgetTop != widgets->begin()) {
        activeWidgetTop--;
        activeWidgetBottom--;
        widgetMap->value(*activeWidgetBottom)->hide();
        if (widgetMap->contains(*activeWidgetTop)) {
            widgetMap->value(*activeWidgetTop)->show();
        }
        else {
            Q_EMIT requestWidget(*activeWidgetTop);
        }
    }
}

void RecyclerView::populateMap()
{

    QList<int>::iterator mapIterator = widgets->begin();

    if (widgetMap->isEmpty()) {
        Q_EMIT requestWidget(*mapIterator);
        ++mapIterator;
    }

    MAX_ROW_COUNT = 1;//this->size().height()/ widgetMap->value(0)->sizeHint().height();

    int i = 0;
    while ( i < MAX_ROW_COUNT && mapIterator != widgets->end()) {
        if (widgetMap->contains(*mapIterator)) {
            widgetMap->value(*mapIterator)->show();
        }
        else {
            Q_EMIT requestWidget(*mapIterator);
        }
        ++mapIterator;
        ++i;
    }

    activeWidgetBottom = mapIterator;
    activeWidgetTop = widgets->begin();

    //TODO use the received data to compute slider maximum
    slider->setMaximum(widgets->length());
    slider->setSingleStep(1);
}

bool RecyclerView::eventFilter(QObject *watched, QEvent *event)
{
    if( event->type() == QScrollEvent::Scroll ) {
        QScrollEvent *scrollEvent = static_cast<QScrollEvent *>(event);
        slider->setSliderPosition(m_scrollBarCurrentValue + 1);
    }

    return QWidget::eventFilter(watched,event);
}



