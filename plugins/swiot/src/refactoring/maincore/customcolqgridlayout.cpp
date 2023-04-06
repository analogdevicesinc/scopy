#include "customcolqgridlayout.hpp"
#include "ui_customcolqgridlayout.h"
#include "QScrollArea"
#include <QSpacerItem>
#include <QtDebug>
#include <iostream>

using namespace adiscope;

CustomColQGridLayout::CustomColQGridLayout(int maxCols, bool hasScrollArea, QWidget *parent) :
	QWidget(parent),
	hasScrollArea(hasScrollArea),
	m_maxCols(maxCols-1),
	currentNumberOfCols(m_maxCols),
        col(0),
	row(0),
	updatePending(false),
	ui(new Ui::CustomColQGridLayout)
{
	ui->setupUi(this);

        m_mainWidget = new QWidget(this);

        this->ui->scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        if (hasScrollArea) { // default
                ui->scrollArea->setWidgetResizable(true);
                ui->scrollArea->setWidget(m_mainWidget);
        } else {
                ui->gridLayout_2->addWidget(m_mainWidget);
                this->ui->scrollArea->hide();
        }

        m_gridLayout = new QGridLayout;
	m_gridLayout->setHorizontalSpacing(0);
	m_gridLayout->setVerticalSpacing(0);
	m_gridLayout->setContentsMargins(0, 0, 0, 0);
	m_gridLayout->setSpacing(0);
	m_gridLayout->setMargin(0);
	m_mainWidget->setLayout(m_gridLayout);

	m_hspacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_vspacer = new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding);

	m_gridLayout->addItem(m_hspacer,0, m_maxCols + 1);
	m_gridLayout->addItem(m_vspacer,10, 0);

	connect(this, &CustomColQGridLayout::reqestLayoutUpdate, this, &CustomColQGridLayout::updateLayout, Qt::QueuedConnection);
}

// adds widget to internal widget list and return the index of the added widget
int CustomColQGridLayout::addQWidgetToList(QWidget *widget)
{
	m_widgetList.push_back(widget);
	return  m_widgetList.size() -1;
}

// adds a widget at index to the layout and it's index to the active widget list
void CustomColQGridLayout::addWidget(int index)
{
	m_gridLayout->addWidget(m_widgetList.at(index),row,col);
	m_widgetList.at(index)->show();
	m_activeWidgetList.push_back(index);

	//logic for resizable n columns grid layout
	if (col == currentNumberOfCols) {
		col = 0;
		row++;
	} else {
		col ++;
	}
}

// remove widget at index from layout
void CustomColQGridLayout::removeWidget(int index)
{
	//get index, row and column of item that will be removed
	int indexOfGrid = m_gridLayout->indexOf(m_widgetList.at(index));
	int r = 0, c=0, rs = 0, cs =0;
	m_gridLayout->getItemPosition(indexOfGrid,&r,&c,&rs,&cs);

	//remove the widget from the layout and hide it (use hide instead of delete since widget can be added again)
	m_gridLayout->removeWidget(m_widgetList.at(index));
	m_widgetList.at(index)->hide();

	int i = 0;
	for (int idx = 0; idx < m_activeWidgetList.size(); idx++) {
		if (m_activeWidgetList.at(idx) == index) {
			i = idx;
			m_activeWidgetList.remove(idx);
			break;
		}
	}

	//shift all remaing active widgets to fill the empty space
	while (i < m_activeWidgetList.size()) {
		repositionWidgets(m_activeWidgetList.at(i),r,c);
		i++;
		if (c == currentNumberOfCols) {
			c = 0;
			r++;
		} else {
			c ++;
		}
	}

	//logic for n columns grid layout
	if (col != 0) {
		col--;
	} else {
		row--;
		col = currentNumberOfCols;
	}
}

// move a widget to specified row and column
// moving the widget will replace the current widget at that position
void CustomColQGridLayout::repositionWidgets(int index, int row, int col)
{
	m_gridLayout->addWidget(m_widgetList.at(index),row,col);
}

// returns widget at index
QWidget* CustomColQGridLayout::getWidget(int index)
{
	return m_widgetList.at(index);
}

// check if widget is active on layout
bool CustomColQGridLayout::isWidgetActive(int index)
{
	for (int i = 0 ; i < m_activeWidgetList.size(); i++) {
		if (m_activeWidgetList.at(i) == index) {
			return true;
		}
	}
	return false;
}

//toggle all widgets
void CustomColQGridLayout::toggleAll(bool toggled)
{
	for (int i=0; i< m_widgetList.size(); i++) {
		if (toggled) {
			addWidget(i);
		} else {
			removeWidget(i);
		}
	}
}

//set the maximum number of columns
void CustomColQGridLayout::setMaxColumnNumber(int maxColumns)
{
	m_maxCols = maxColumns - 1;
}

//returns the maximum number of columns
int CustomColQGridLayout::getMaxColumnNumber()
{
	return  m_maxCols;
}

//returns the number of rows that contain the maximum number of cols/elements that could fit on a row
int CustomColQGridLayout::fullRows() const
{
        return row;
}

//returns the number of rows, including the last row that might not be full
int CustomColQGridLayout::rows() const
{
        return row + (col != 0);
}

//returns the maximum number of cols/elements that are currently on a row
int CustomColQGridLayout::columns() const
{
        if (row == 0) return 0;
        return ((int) (m_widgetList.size() - col)) / row;
}

//returns the number of cols/elements that are on the last row
int CustomColQGridLayout::columnsOnLastRow() const
{
        return col;
}

// returns the number of widgets in the grid
int CustomColQGridLayout::count() const
{
	return this->m_widgetList.size();
}

void CustomColQGridLayout::resizeEvent(QResizeEvent *event)
{
	if (!updatePending) {
		updatePending = true;
		Q_EMIT reqestLayoutUpdate();
	}
}

void CustomColQGridLayout::itemSizeChanged()
{
	if (!updatePending) {
		updatePending = true;
		Q_EMIT reqestLayoutUpdate();
	}
}

void CustomColQGridLayout::updateLayout()
{
	if (availableWidth != this->width()) {
		if (m_activeWidgetList.size() > 0) {
			computeCols(m_widgetList.at(m_activeWidgetList.at(0))->minimumSizeHint().width());
		}
	} else {
		recomputeColCount();
	}
	updatePending = false;
}

void CustomColQGridLayout::recomputeColCount()
{
	if (m_activeWidgetList.size() > 0) {
		auto maxWidth = m_widgetList.at(m_activeWidgetList.at(0))->minimumSizeHint().width();
		for (int i = 1; i < m_activeWidgetList.size(); i++ ) {
			if (m_widgetList.at(m_activeWidgetList.at(i))->minimumSizeHint().width()> maxWidth){
				maxWidth = m_widgetList.at(m_activeWidgetList.at(i))->minimumSizeHint().width();
			}
		}
		if (colWidth != maxWidth) {
			computeCols(maxWidth);
		}
	} else {
		computeCols(0);
	}
}

void CustomColQGridLayout::computeCols(double width) // width of the first active widget
{
	int colCount = currentNumberOfCols;
	availableWidth = this->width();

	if (width != 0) {
		colCount = (availableWidth / width) - 1; //index starts at 0
		int width_extra = availableWidth - (colCount+1) * width;
		colWidth = width + (width_extra/(colCount+1));

	} else {
		if (m_widgetList.size() > 0) {
			if (m_widgetList.at(0)->width() > 0) {
				colCount = (availableWidth / m_widgetList.at(0)->width()) - 1;
				colWidth = m_widgetList.at(0)->width();
			}
		}
	}

	if (colCount != currentNumberOfCols) {
		currentNumberOfCols = colCount;

		if (m_activeWidgetList.size() > 1) {
			redrawWidgets();
		}
	}
}

// redraw all active widgets
void CustomColQGridLayout::redrawWidgets()
{
	row = 0;
	col = 0;
	if (!m_activeWidgetList.empty()) {
		for (int i = 0; i < m_activeWidgetList.size(); i++) {

			m_gridLayout->removeWidget(m_widgetList.at(m_activeWidgetList.at(i)));
			m_widgetList.at(m_activeWidgetList.at(i))->hide();

			m_gridLayout->addWidget(m_widgetList.at(m_activeWidgetList.at(i)),row,col);
			m_widgetList.at(m_activeWidgetList.at(i))->show();

			//logic for resizable n columns grid layout
			if (col == currentNumberOfCols) {
				col = 0;
				row++;
			} else {
				col++;
			}
		}
	}

        if (!hasScrollArea) {
                m_gridLayout->setRowMinimumHeight(0, this->m_widgetList.at(0)->height());
                this->setMinimumHeight(this->rows() * this->m_widgetList.at(0)->height());
                Q_EMIT reqestLayoutUpdate();
        }
}

CustomColQGridLayout::~CustomColQGridLayout()
{
	for (auto widget: m_widgetList) {
		delete widget;
	}
	if (m_gridLayout) {
		m_gridLayout->removeItem(m_vspacer);
		m_gridLayout->removeItem(m_hspacer);
		delete m_gridLayout;
	}
	delete ui;
}
