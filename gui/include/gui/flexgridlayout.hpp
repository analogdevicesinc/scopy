#ifndef FLEXGRIDLAYOUT_HPP
#define FLEXGRIDLAYOUT_HPP

#include "scopy-gui_export.h"

#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

namespace scopy {
class SCOPY_GUI_EXPORT FlexGridLayout : public QWidget
{
	Q_OBJECT

public:
	explicit FlexGridLayout(int maxCols, QWidget *parent = nullptr);
	~FlexGridLayout() override;

	void toggleAll(bool toggled);
	void addWidget(int index);
	void removeWidget(int index);
	QWidget *getWidget(int index);
	bool isWidgetActive(int index);
	int addQWidgetToList(QWidget *widget);

	void setMaxColumnNumber(int maxColumns);
	int getMaxColumnNumber();

	int fullRows() const;
	int rows() const;
	int columns() const;
	int columnsOnLastRow() const;

	int count() const;

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
	double availableWidth;
	std::vector<QWidget *> m_widgetList;
	QVector<int> m_activeWidgetList;
	QGridLayout *m_gridLayout;
	QSpacerItem *m_hspacer;
	QSpacerItem *m_vspacer;

	// resize related
	bool updatePending;
	void redrawWidgets();
	void recomputeColCount();
	void computeCols(double width);
	void resizeEvent(QResizeEvent *event) override;
	void repositionWidgets(int index, int row, int col);
};
} // namespace scopy
#endif // FLEXGRIDLAYOUT_HPP
