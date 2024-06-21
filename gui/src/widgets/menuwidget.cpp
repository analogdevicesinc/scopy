#include "menuwidget.h"
#include <QScrollArea>
#include <menuheader.h>

using namespace scopy;
using namespace scopy::gui;

MenuWidget::MenuWidget(QString name, QPen p, QWidget *parent)
	: QWidget(parent)
{
	uuid = 0;
	QVBoxLayout *lay = new QVBoxLayout();

	QScrollArea *scroll = new QScrollArea(parent);
	QWidget *wScroll = new QWidget(scroll);

	m_layScroll = new QVBoxLayout();
	m_layScroll->setMargin(0);
	m_layScroll->setSpacing(10);

	wScroll->setLayout(m_layScroll);
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	// if ScrollBarAlwaysOn - layScroll->setContentsMargins(0,0,6,0);

	scroll->setWidget(wScroll);

	lay->setMargin(0);
	lay->setSpacing(10);
	setLayout(lay);

	m_spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	m_layScroll->addSpacerItem(m_spacer);

	m_header = new MenuHeaderWidget(name, p, this);
	lay->addWidget(m_header);
	lay->addWidget(scroll);
}

MenuWidget::~MenuWidget() {}

void MenuWidget::add(QWidget *w, QString name, MenuAlignment position)
{
	m_widgetMap.insert(name, w);
	int spacerIndex = m_layScroll->indexOf(m_spacer);
	switch(position) {

	case MA_TOPFIRST:
		m_layScroll->insertWidget(0, w);
		break;
	case MA_TOPLAST:
		m_layScroll->insertWidget(spacerIndex, w);
		break;
	case MA_BOTTOMFIRST:
		m_layScroll->insertWidget(spacerIndex + 1, w);
		break;
	case MA_BOTTOMLAST:
		m_layScroll->insertWidget(-1, w);
		break;
	}
}

void MenuWidget::add(QWidget *w)
{
	add(w, "widget" + QString(uuid), MA_TOPLAST);
	uuid++;
}

void MenuWidget::add(QWidget *w, QString name) { add(w, name, MA_TOPLAST); }

void MenuWidget::remove(QWidget *w)
{
	m_widgetMap.remove(widgetName(w));
	m_layScroll->removeWidget(w);
}

void MenuWidget::remove(QString s)
{
	QWidget *w = findWidget(s);
	m_widgetMap.remove(s);
	m_layScroll->removeWidget(w);
}

MenuHeaderWidget *MenuWidget::header() { return m_header; }

QWidget *MenuWidget::findWidget(QString name) { return m_widgetMap.value(name, nullptr); }

QString MenuWidget::widgetName(QWidget *w) { return m_widgetMap.key(w, ""); }

void MenuWidget::collapseAll()
{
	for(QWidget *w : m_widgetMap.values()) {
		Collapsable *c = dynamic_cast<Collapsable *>(w);
		if(c != nullptr) {
			c->setCollapsed(true);
		}
	}
}

void MenuWidget::setCollapsed(QString name, bool b)
{
	QWidget *w = findWidget(name);
	Collapsable *c = dynamic_cast<Collapsable *>(w);
	if(c != nullptr) {
		c->setCollapsed(b);
	}
}

#include "moc_menuwidget.cpp"
