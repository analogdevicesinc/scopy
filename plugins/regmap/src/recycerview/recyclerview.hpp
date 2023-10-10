#ifndef RECYCLERVIEW_H
#define RECYCLERVIEW_H

#include <QMap>
#include <QObject>
#include <QWidget>

#include <src/verticalscrollarea.hpp>

class QSlider;
class QGridLayout;

class QHBoxLayout;
namespace scopy::regmap {
class RecyclerView : public QWidget
{
	Q_OBJECT

public:
	explicit RecyclerView(QList<int> *widgets, QWidget *parent = nullptr);
	~RecyclerView();

	QMap<int, QWidget *> *getWidgetsMap() const;
	void setWidgetMap(QMap<int, QWidget *> *newWidgets);
	void addWidget(int index, QWidget *widget);
	QWidget *getWidgetAtIndex(int index);

	void hideAll();
	void showAll();
	void setActiveWidgets(QList<int> *widgets);
	void scrollTo(int index);
	void setMaxrowCount(int maxRowCount);

	void init();
	void populateMap();

Q_SIGNALS:
	void requestWidget(int index);
	void initDone();
	void requestInit();

private:
	int maxRowCount;
	QSlider *slider;
	VerticalScrollArea *m_scrollArea;
	int m_scrollBarCurrentValue;

	QGridLayout *bitFieldsWidgetLayout;
	QHBoxLayout *layout;

	QMap<int, QWidget *> *widgetMap;
	QList<int> *widgets;
	QList<int>::iterator activeWidgetTop;
	QList<int>::iterator activeWidgetBottom;

	void scrollDown();
	void scrollUp();

	// QObject interface
public:
	bool eventFilter(QObject *watched, QEvent *event);
};
} // namespace scopy::regmap
#endif // RECYCLERVIEW_H
