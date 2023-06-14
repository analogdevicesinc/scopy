#include "recyclerview.hpp"
#include <QGridLayout>
#include <QSlider>
#include <QScrollBar>
#include <qlabel.h>
#include <QDebug>
#include <QEvent>
#include <QScrollEvent>
#include <src/verticalscrollarea.hpp>

using namespace scopy::regmap::gui;

RecyclerView::RecyclerView(QList<int> *widgets,QWidget *parent)
    : widgets(widgets),
    QWidget{parent}
{
    this->installEventFilter(this);
    this->setLayout(new QHBoxLayout());
    layout()->setMargin(0);
    layout()->setSpacing(0);

    bitFieldsWidgetLayout = new QGridLayout();
    bitFieldsWidgetLayout->setMargin(0);
    bitFieldsWidgetLayout->setSpacing(0);
    QWidget *bitFieldsWidget = new QWidget();
    bitFieldsWidget->setLayout(bitFieldsWidgetLayout);

    m_scrollArea = new VerticalScrollArea();
    m_scrollArea->setWidget(bitFieldsWidget);
    m_scrollArea->verticalScrollBar()->setVisible(false);
    this->layout()->addWidget(m_scrollArea);

    slider = new QSlider();
    slider->setInvertedAppearance(true);
    slider->setInvertedControls(true);

    QObject::connect(m_scrollArea->verticalScrollBar(),&QAbstractSlider::valueChanged, this, [=](int value){
        if (value == m_scrollArea->verticalScrollBar()->minimum()) {
            slider->setValue(m_scrollBarCurrentValue - 1);
            m_scrollArea->verticalScrollBar()->setValue(value + 1 );
        }
        if (value == m_scrollArea->verticalScrollBar()->maximum()){
            slider->setValue(m_scrollBarCurrentValue + 1);
            m_scrollArea->verticalScrollBar()->setValue(value - 1 );
        }

    });
    this->layout()->addWidget(slider);
}

RecyclerView::~RecyclerView()
{
    delete bitFieldsWidgetLayout;
    delete widgetMap;
    delete m_scrollArea;
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

void RecyclerView::scrollTo(int index)
{
    m_scrollArea->verticalScrollBar()->setValue(0);
    slider->setValue(index);
}

void RecyclerView::setMaxrowCount(int maxRowCount)
{
    this->maxRowCount = maxRowCount;
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

    //TODO find a way to autocompute max row count
    maxRowCount = 10;

    int i = 0;
    while ( i < maxRowCount && mapIterator != widgets->end()) {
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



