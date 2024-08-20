#ifndef PLOTBUTTONMANAGER_H
#define PLOTBUTTONMANAGER_H
#include "utils.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <compositewidget.h>

class PlotButtonManager : public QWidget, public CompositeWidget, public Collapsable
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	PlotButtonManager(QWidget *parent);
	~PlotButtonManager();

	void add(QWidget *w) override;
	void remove(QWidget *w) override;
	bool event(QEvent *ev) override;

public Q_SLOTS:
	virtual bool collapsed() override;
	virtual void setCollapsed(bool b) override;

private:
	void collapsePriv(bool);
	QHBoxLayout *m_lay;
	QPushButton *m_collapse;
	QMap<QWidget*, QWidget*> m_map;

};

#endif // PLOTBUTTONMANAGER_H

