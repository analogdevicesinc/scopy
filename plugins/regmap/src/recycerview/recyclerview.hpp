#ifndef RECYCLERVIEW_H
#define RECYCLERVIEW_H

#include <QObject>
#include <QWidget>
#include <QMap>

class QSlider;
class QGridLayout;
class RecyclerView : public QWidget
{
    Q_OBJECT

public:
    explicit RecyclerView(QList<int> *widgets, QWidget *parent = nullptr);
    ~RecyclerView();

    QMap<int, QWidget*> *getWidgetsMap() const;
    void setWidgetMap(QMap<int, QWidget*> *newWidgets);
    void addWidget(int index, QWidget *widget);
    QWidget* getWidgetAtIndex(int index);

    void hideAll();
    void showAll();
    void setActiveWidgets(QList<int> *widgets);

    void init();
    void populateMap();

Q_SIGNALS:
    void requestWidget(int index);

private:
    int MAX_ROW_COUNT;
    QSlider *slider ;
    int m_scrollBarCurrentValue;

    QGridLayout *bitFieldsWidgetLayout;

    QMap<int, QWidget*> *widgetMap;
    QList<int> *widgets;
    QList<int>::iterator activeWidgetTop;
    QList<int>::iterator activeWidgetBottom;

    void scrollDown();
    void scrollUp();



    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // RECYCLERVIEW_H
