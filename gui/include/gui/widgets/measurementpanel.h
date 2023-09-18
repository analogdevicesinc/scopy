#ifndef MEASUREMENTPANEL_H
#define MEASUREMENTPANEL_H

#include <QWidget>
#include <QScrollBar>
#include "scopy-gui_export.h"
#include "utils.h"
#include <QGridLayout>

namespace Ui {
class MeasurementsPanel;
};

namespace scopy::gui {

class RowColumnWrappingWidget : public QWidget {
	Q_OBJECT
public:
	typedef enum {
		HORIZONTAL,
		VERTICAL
	} WrappingDirection;

	RowColumnWrappingWidget(QWidget *parent = nullptr) {
		m_maxCount = 4;
		m_dir = HORIZONTAL;
		m_grid = new QGridLayout(this);
		setLayout(m_grid);
		m_grid->setSpacing(0);
		m_grid->setMargin(0);
		m_grid->setAlignment(Qt::AlignTop);
		setFixedHeight(parent->height());
		m_grid->addItem(new QSpacerItem(10,10,QSizePolicy::Minimum, QSizePolicy::Expanding),m_maxCount,0);

	}


	void setWrappingDirection(int count, WrappingDirection dir) {
		m_maxCount = count;
		m_dir = dir;
		update();
	}
	void addWidget(QWidget *w) {
		m_list.push_back(w);		
		m_grid->addWidget(w, m_rowCount, m_columnCount);
		switch(m_dir) {
		case HORIZONTAL:
			m_rowCount++;
			if(m_rowCount == m_maxCount) {
				m_columnCount++;
				m_rowCount = 0;
			}

			break;
		case VERTICAL:
		default:
			m_columnCount++;
			if(m_columnCount < m_maxCount) {
				m_rowCount++;
				m_columnCount = 0;
			}
			break;
		}
	}

	void removeWidget(QWidget *w) {
		m_list.removeAll(w);
		w->setParent(nullptr);
		update();
	}

public Q_SLOTS:
	void update() {
		for(QWidget *w : m_list) {
			addWidget(w);
		}
	}

	void reset() {
		m_list.clear();
		m_rowCount = 0;
		m_columnCount = 0;
		update();
	}

private:
	int m_maxCount;
	int m_rowCount;
	int m_columnCount;
	QGridLayout *m_grid;
	WrappingDirection m_dir;
	QList<QWidget*> m_list;
};

class SCOPY_GUI_EXPORT MeasurementsPanel : public QWidget
{
	Q_OBJECT
public:
	MeasurementsPanel(QWidget *parent = nullptr);
	QWidget *cursorArea();

public Q_SLOTS:
	void addMeasurement(QWidget *meas);
	void removeMeasurement(QWidget *meas);
	void update();
	void sort();
private:
	QList<QWidget*> m_labels;
	Ui::MeasurementsPanel *ui;
	RowColumnWrappingWidget *wrap;
};

class SCOPY_GUI_EXPORT StatsPanel : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	StatsPanel(QWidget *parent = nullptr);
	~StatsPanel();
public Q_SLOTS:
	void addStat(QWidget *stat);
	void removeStat(QWidget *stat);
	void clear();
	void sort();
private:
	QHBoxLayout *panelLayout;
	QList<QWidget*> m_labels;


};
}
#endif // MEASUREMENTPANEL_H
