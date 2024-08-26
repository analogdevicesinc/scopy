#include "plotbuttonmanager.h"

PlotButtonManager::PlotButtonManager(QWidget *parent)
	: QWidget(parent)
{

	m_collapsableContainer = new QWidget(this);
	m_collapsablelay = new QHBoxLayout(m_collapsableContainer);
	m_collapsablelay->setSpacing(0);
	m_collapsablelay->setMargin(0);
	m_collapsableContainer->setLayout(m_collapsablelay);

	m_lay = new QHBoxLayout(this);
	m_collapseBtn = new QPushButton("", this);
	m_collapseBtn->setCheckable(true);
	m_collapseBtn->setChecked(true);
	m_collapseBtn->setFixedSize(4, 16);
	m_collapseBtn->setStyleSheet("background-color: #AAAAAAAA");

	m_lay->addWidget(m_collapseBtn);
	m_lay->addWidget(m_collapsableContainer);
	m_lay->setSpacing(0);
	m_lay->setMargin(0);

	connect(m_collapseBtn, &QAbstractButton::toggled, this, &PlotButtonManager::collapsePriv);

	setMouseTracking(true);
	m_collapsableContainer->setVisible(false);
	m_collapseBtn->setVisible(false);
}

PlotButtonManager::~PlotButtonManager() {}

void PlotButtonManager::add(QWidget *w)
{
	m_collapseBtn->setVisible(m_collapsablelay->count() > 0);
	m_collapsablelay->addWidget(w);
}

void PlotButtonManager::remove(QWidget *w)
{
	m_collapseBtn->setVisible(m_collapsablelay->count() > 0);
	m_collapsablelay->removeWidget(w);
}

bool PlotButtonManager::event(QEvent *event)
{
	if(event->type() == QEvent::Enter) {
		collapsePriv(false);
	}
	if(event->type() == QEvent::Leave) {
		collapsePriv(true);
	}

	return QWidget::event(event);
}

void PlotButtonManager::setCollapseOrientation(CollapseButtonOrientation p)
{
	if(p == PBM_LEFT) {
		m_lay->addWidget(m_collapseBtn);
		m_lay->addWidget(m_collapsableContainer);
	} else {
		m_lay->addWidget(m_collapsableContainer);
		m_lay->addWidget(m_collapseBtn);
	}
}

bool PlotButtonManager::collapsed() { return m_collapseBtn->isChecked(); }

void PlotButtonManager::setCollapsed(bool b) { m_collapseBtn->setChecked(true); }

void PlotButtonManager::collapsePriv(bool b) { m_collapsableContainer->setVisible(!b); }

// eventfilter

#include "moc_plotbuttonmanager.cpp"
