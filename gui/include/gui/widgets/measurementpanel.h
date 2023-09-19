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

namespace scopy {

class VerticalWidgetStack : public QWidget {
	Q_OBJECT
public:
	VerticalWidgetStack(QWidget *parent = nullptr) {
		lay = new QVBoxLayout(this);
		setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Expanding);
		setLayout(lay);
		lay->setMargin(0);
		lay->setSpacing(6);
		spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
		lay->addSpacerItem(spacer);
		stackSize = 4;
	}
	~VerticalWidgetStack() {

	}
	void addWidget(QWidget *w) {
		int idx = lay->indexOf(spacer);
		lay->insertWidget(idx,w,Qt::AlignTop | Qt::AlignLeft);
		m_widgets.append(w);
	}

	void setStackSize(int val) {
		stackSize = val;
	}

	void reparentWidgets(QWidget *parent = nullptr) {
		for(QWidget *w : m_widgets) {
			lay->removeWidget(w);
			w->setParent(parent);

		}
		m_widgets.clear();
	}

	bool full() {
		return(lay->count() > stackSize);
	}

private:
	QVBoxLayout *lay;
	QSpacerItem *spacer;
	int stackSize;
	QList<QWidget*> m_widgets;
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
	QHBoxLayout *panelLayout;
	QList<QWidget*> m_labels;
	QList<VerticalWidgetStack*> m_stacks;
	QWidget *m_cursor;
	QSpacerItem *spacer;
	void addWidget(QWidget *meas);
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
