#ifndef CUSOTMCOLQGRIDLAYOUT_HPP
#define CUSOTMCOLQGRIDLAYOUT_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QTimer>

namespace Ui {
class CustomColQGridLayout;
}

namespace adiscope {
class CustomColQGridLayout : public QWidget
{
	Q_OBJECT

public:
	explicit CustomColQGridLayout(int maxCols, QWidget *parent = nullptr);
	~CustomColQGridLayout();

	void toggleAll(bool toggled);
	void addWidget(int index);
	void removeWidget(int index);
	QWidget* getWidget(int index);
	bool isWidgetActive(int index);
	int addQWidgetToList(QWidget *widget);

	void setMaxColumnNumber(int maxColumns);
	int getMaxColumnNumber();

Q_SIGNALS:
	void reqestLayoutUpdate();

public Q_SLOTS:
	void updateLayout();
	void itemSizeChanged();

private:
	int m_maxCols;
	int currentNumberOfCols;
	int col;
	int row;
	double colWidth;
	std::vector<QWidget*> m_widgetList;
	QVector<int> m_activeWidgetList;
	QGridLayout *m_gridLayout;
	QWidget *m_mainWidget;
	QSpacerItem *m_hspacer;
	QSpacerItem *m_vspacer;

	//resize related
	bool updatePending;
	void redrawWidgets();
	void recomputeColCount();
	void computeCols(double width);
	void resizeEvent(QResizeEvent *event);
	void repositionWidgets(int index, int row, int col);

	Ui::CustomColQGridLayout *ui;
};
}
#endif // CUSOTMCOLQGRIDLAYOUT_HPP
