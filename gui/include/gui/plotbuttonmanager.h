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
	typedef enum {
		PBM_LEFT,
		PBM_RIGHT
	} CollapseButtonOrientation;
	PlotButtonManager(QWidget *parent);
	~PlotButtonManager();

	void add(QWidget *w) override;
	void remove(QWidget *w) override;
	bool event(QEvent *ev) override;
	void setCollapseOrientation(CollapseButtonOrientation);

public Q_SLOTS:
	virtual bool collapsed() override;
	virtual void setCollapsed(bool b) override;

private:
	void collapsePriv(bool);
	QHBoxLayout *m_lay;
	QHBoxLayout *m_collapsablelay;
	QPushButton *m_collapseBtn;
	QWidget *m_collapsableContainer;

};

#endif // PLOTBUTTONMANAGER_H

