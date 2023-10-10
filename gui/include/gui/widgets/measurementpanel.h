#ifndef MEASUREMENTPANEL_H
#define MEASUREMENTPANEL_H

#include "scopy-gui_export.h"
#include "utils.h"

#include <QGridLayout>
#include <QScrollBar>
#include <QWidget>

namespace Ui {
class MeasurementsPanel;
};

namespace scopy {
class MeasurementLabel;
class StatsLabel;

class VerticalWidgetStack : public QWidget
{
	Q_OBJECT
public:
	VerticalWidgetStack(QWidget *parent = nullptr)
	{
		lay = new QVBoxLayout(this);
		setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
		setLayout(lay);
		lay->setMargin(0);
		lay->setSpacing(6);
		spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
		lay->addSpacerItem(spacer);
		stackSize = 4;
	}
	~VerticalWidgetStack() {}
	void addWidget(QWidget *w)
	{
		int idx = lay->indexOf(spacer);
		lay->insertWidget(idx, w, Qt::AlignTop | Qt::AlignLeft);
		m_widgets.append(w);
	}

	void setStackSize(int val) { stackSize = val; }

	void reparentWidgets(QWidget *parent = nullptr)
	{
		for(QWidget *w : m_widgets) {
			lay->removeWidget(w);
			w->setParent(parent);
		}
		m_widgets.clear();
	}

	bool full() { return (lay->count() > stackSize); }

private:
	QVBoxLayout *lay;
	QSpacerItem *spacer;
	int stackSize;
	QList<QWidget *> m_widgets;
};

class SCOPY_GUI_EXPORT MeasurementsPanel : public QWidget
{
	Q_OBJECT
public:
	MeasurementsPanel(QWidget *parent = nullptr);
	QWidget *cursorArea();

public Q_SLOTS:
	void addMeasurement(MeasurementLabel *meas);
	void removeMeasurement(MeasurementLabel *meas);
	void updateOrder();
	void sort(int sortType); // hackish

private:
	QHBoxLayout *panelLayout;
	QList<MeasurementLabel *> m_labels;
	QList<VerticalWidgetStack *> m_stacks;
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
	void addStat(StatsLabel *stat);
	void removeStat(StatsLabel *stat);
	void updateOrder();
	void sort(int sortType); // hackish
private:
	QHBoxLayout *panelLayout;
	QList<StatsLabel *> m_labels;
};
} // namespace scopy
#endif // MEASUREMENTPANEL_H
