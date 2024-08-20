#include "plotbuttonmanager.h"

PlotButtonManager::PlotButtonManager(QWidget *parent) : QWidget(parent)
{
	m_lay = new QHBoxLayout(this);
	m_collapse = new QPushButton("",this);
	m_collapse->setCheckable(true);
	m_collapse->setChecked(true);

	m_collapse->setFixedSize(8,16);
	m_lay->addWidget(m_collapse);
	m_lay->setSpacing(0);
	m_lay->setMargin(0);

	connect(m_collapse,&QAbstractButton::toggled, this, &PlotButtonManager::collapsePriv);

	setMouseTracking(true);
}

PlotButtonManager::~PlotButtonManager()
{

}

void PlotButtonManager::add(QWidget *w)
{
	// a trick to interact nicely with plotButtonManager - we put the widget in a container widget
	// we control widget visiblity separately and plotButtonmanager controls container visibility separately

	QWidget *ww = new QWidget(this);
	QHBoxLayout *lay = new QHBoxLayout(ww);
	lay->setSpacing(0);
	lay->setMargin(0);
	lay->addWidget(w);

	m_lay->addWidget(ww);
	m_map[w] = ww;
	ww->setVisible(!m_collapse->isChecked());
}

void PlotButtonManager::remove(QWidget *w)
{
	m_map[w]->layout()->removeWidget(w);
	m_lay->removeWidget(m_map[w]);
	delete m_map[w];
	m_map.remove(w);
}

bool PlotButtonManager::event(QEvent *event)
{
	if (event->type() == QEvent::Enter) {
		collapsePriv(false);

	} else if (event->type() == QEvent::Leave) {
		collapsePriv(true);
	}

	return QWidget::event(event);
}

bool PlotButtonManager::collapsed()
{
	return m_collapse->isChecked();
}

void PlotButtonManager::setCollapsed(bool b)
{
	m_collapse->setChecked(true);
}

void PlotButtonManager::collapsePriv(bool b)
{
	for(int i = 0;i<m_lay->count();i++){
		QWidget *w = dynamic_cast<QWidget*>(m_lay->itemAt(i)->widget());
		if(w) {
			if(w == m_collapse)
				continue;
			w->setVisible(!b);
		}
	}

}

// eventfilter






